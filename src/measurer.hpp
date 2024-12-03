#pragma once
#include <xtd/console.h>
#include <xtd/ustring.h>

#include <vector>
#include <thread>
#include <chrono>
#include <format>
#include <filesystem>
#include <numeric>
#include <unordered_map>

#include "hardware/meter.hpp"

namespace chrono = std::chrono;
namespace fs = std::filesystem;

struct Measurement {
	chrono::duration<double, std::ratio<1>> time;
	std::vector<double> values;
};

std::ostream& operator<<(std::ostream& os, const Measurement& meas) {
	return os << std::accumulate(
		meas.values.cbegin(),
		meas.values.cend(),
		std::to_string(meas.time.count()),
		[](const std::string& prev, double val) {
			return std::format("{}\t{}", prev, val);
		}
	);
}

class Measurer {

	const Meter::List meters;
	fs::path directory;
	fs::path fname;
	double timeout;
	chrono::steady_clock::time_point startTime;

public:
	Measurer(Meter::List&& meters_, const std::string& directory_, const double timeout_)
		: meters{ std::move(meters_) }
		, directory{ directory_ }
		, fname{ std::format(
			"MEASURER_DATA_{0:%F}_{0:%H-%M-%S}.csv",
			chrono::floor<chrono::seconds>(chrono::current_zone()->to_local(chrono::system_clock::now()))
		) }
		, timeout{ timeout_ }
	{
		lg::info("measurer will save date into file '{}' with timeout t = {}s", fname.string(), timeout);
	}

	void run() {
		if (meters.empty()) {
			lg::warn("There is no meters to measure!");
			return;
		}

		createFile();
		std::ofstream file(directory / fname, std::ios::app);

		preRun();
		while (true) {
			if (xtd::console::key_available()) {
				auto key = xtd::console::read_key();
				if (key.modifiers() == xtd::console_modifiers::control && key.key() == xtd::console_key::q)
					return;
			}

			auto meas = processMeters();
			file << meas << std::endl;
			std::this_thread::sleep_for(chrono::duration<double>(timeout));
		}
	}
private:
	void createFile() {
		if (!fs::exists(directory))
			fs::create_directory(directory);

		std::ofstream file(directory / fname, std::ios::out);
		auto header = std::accumulate(
			meters.cbegin(),
			meters.cend(),
			std::string("time,s"),
			[](const std::string& prev, const std::unique_ptr<Meter>& meter) {
				return std::format("{}\t{},{}", prev, meter->name(), meter->type()._to_string());
			}
		);
		file << header << std::endl;
	}

	void preRun() {
		xtd::console::foreground_color(xtd::console_color::blue);
		xtd::console::write_line("Measurements started at {}", chrono::system_clock::now());
		xtd::console::write_line("Press CTRL+Q to stop measures");
		xtd::console::reset_color();

		startTime = chrono::steady_clock::now();
	}

	Measurement processMeters() {
		std::vector<std::thread> threads;
		std::unordered_map<Hard::Name, double> values;
		std::mutex mu;

		auto curTime = chrono::steady_clock::now();
		for (const auto& meter : meters) {
			threads.emplace_back([&meter, &values, &mu]() {
				auto val = meter->value();
				std::lock_guard<std::mutex> lg{ mu };
				values[meter->name()] = val;
			});
		}
		for (auto& th : threads)
			th.join();

		std::vector<double> res;
		for (const auto& meter : meters)
			res.push_back(values[meter->name()]);

		return { curTime - startTime, res };
	}
};