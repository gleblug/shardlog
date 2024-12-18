#include "measurer.hpp"

#include <thread>
#include <format>
#include <numeric>
#include <unordered_map>
#include <fstream>

#include <xtd/console.h>
#include <xtd/ustring.h>
#include <fmt/std.h>

#include "../meter/meter.hpp"
#include "setter.hpp"

using xtd::ustring;
using xtd::console;

std::ostream& operator<<(std::ostream& os, const Measurement& meas) {
	return os << std::accumulate(
		meas.values.cbegin(),
		meas.values.cend(),
		std::format("{:.3f}", meas.fromStart.count()),
		[](const std::string& prev, const std::string& val) {
			return std::format("{}\t{}", prev, ustring(val).trim().c_str());
		}
	);
}
Measurer::Measurer(const std::vector<Meter::Ptr>& meters, const fs::path& directory, const TimeDuration& duration, const TimeDuration& timeout)
	: m_meters{ meters }
	, m_path{ fs::path(directory) / std::format(
		"DATA_{0:%F}_{0:%H-%M-%S}.csv",
		chrono::floor<chrono::seconds>(chrono::current_zone()->to_local(chrono::system_clock::now()))
	) }
	, m_duration{ duration }
	, m_timeout{ (timeout.count() > 0.05) ? timeout : TimeDuration(0.05) }
	, m_counter{ 0 }
	, m_thread{}
	, m_mu{}
{
	if (!fs::exists(m_path.parent_path()))
		fs::create_directory(m_path.parent_path());
	std::ofstream(m_path, std::ios::out);

	const auto width = 24;
	xtd::console::write_line(fmt::format(
		"Measurer configs:\n{:<{}}{}\n{:<{}}{}\n{:<{}}{}",
		"  - save data into", width, m_path,
		"  - duration", width, m_duration,
		"  - timeout", width, m_timeout
	));
}

Measurer::~Measurer() {
	if (m_thread.joinable())
		m_thread.join();
}

void Measurer::start() {
	if (m_meters.empty()) {
		lg::warn("There is no meters to measure!");
		return;
	}
	
	std::ofstream file(m_path, std::ios::app);
	file << header() << std::endl;

	xtd::console::write_line("Measurements started at {}", chrono::system_clock::now());
	xtd::console::write_line("Press CTRL+Q to stop measures");

	loop(file);

	xtd::console::write_line("Average response time:");
	for (const auto& meter : m_meters) {
		xtd::console::write_line(fmt::format(
			"  - {:<20}{}",
			meter->name(),
			chrono::duration_cast<chrono::milliseconds>(meter->averageResponseTime())
		));
	}
}

std::string Measurer::header() const {
	return std::accumulate(
		m_meters.cbegin(),
		m_meters.cend(),
		std::string("time,s"),
		[](const std::string& prev, const Meter::Ptr meter) {
			auto readTitles = meter->readTitles();
			return std::accumulate(
				readTitles.cbegin(), readTitles.cend(), prev,
				[&meter](const std::string& prev, const std::string& title) { return std::format("{}\t{},{}", prev, meter->name(), title); }
			);
		}
	);
	
}

void Measurer::measure() {
	std::vector<std::thread> threads;
	std::mutex mu;
	std::unordered_map<Meter::Name, Meter::Values> values;
	
	auto curTime = chrono::steady_clock::now();
	for (const auto& meter : m_meters) {
		threads.emplace_back([&meter, &values, &mu, this]() {
			meter->readUntil(m_start + chrono::duration_cast<chrono::nanoseconds>(m_timeout) * m_counter - chrono::milliseconds(10));
			std::lock_guard lg(mu);
			values[meter->name()] = meter->get();
		});
	}
	for (auto& th : threads)
		th.join();

	std::vector<std::string> res;
	for (const auto& meter : m_meters) {
		auto val = values.at(meter->name());
		for (const auto& title : meter->readTitles())
			res.push_back(val.at(title));
	}
	std::lock_guard lg(m_mu);
	m_meas = Measurement{ curTime - m_start, res };
}

void Measurer::loop(std::ostream& os) {
	m_start = chrono::steady_clock::now();

	m_thread = std::thread(&Measurer::measure, this);
	m_counter = 1;

	Setter setter(m_meters);
	setter.start();

	while (true) {
		if (chrono::steady_clock::now() - m_start > m_duration)
			break;

		if (xtd::console::key_available()) {
			auto key = xtd::console::read_key();
			if (key.modifiers() == xtd::console_modifiers::control && key.key() == xtd::console_key::q) {
				xtd::console::write_line();
				break;
			}
		}

		if (chrono::steady_clock::now() - m_start >= m_timeout * m_counter) {
			++m_counter;
			m_thread.join();
			std::lock_guard lg(m_mu);
			m_thread = std::thread(&Measurer::measure, this);
			os << m_meas << std::endl;
		}
	}

	setter.stop();
	m_thread.join();
}