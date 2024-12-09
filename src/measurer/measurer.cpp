#include "measurer.hpp"

#include <thread>
#include <format>
#include <numeric>
#include <unordered_map>
#include <fstream>

#include <xtd/console.h>
#include <xtd/ustring.h>

#include "../meter/meter.hpp"

std::ostream& operator<<(std::ostream& os, const Measurement& meas) {
	return os << std::accumulate(
		meas.values.cbegin(),
		meas.values.cend(),
		std::format("{:.3f}", meas.time.count()),
		[](const std::string& prev, const std::string& val) {
			return std::format("{}\t{}", prev, xtd::ustring(val).trim().c_str());
		}
	);
}
Measurer::Measurer(Meter::List&& meters, const std::string& directory, const double timeout)
	: m_meters{ std::move(meters) }
	, m_path{ fs::path(directory) / std::format(
		"DATA_{0:%F}_{0:%H-%M-%S}.csv",
		chrono::floor<chrono::seconds>(chrono::current_zone()->to_local(chrono::system_clock::now()))
	) }
	, m_timeout{ (timeout > 0.05) ? timeout : 0.05 }
	, m_thread{}
	, m_mu{}
{
	if (!fs::exists(m_path.parent_path()))
		fs::create_directory(m_path.parent_path());
	lg::info("measurer will save date into file '{}' with timeout t = {}s", m_path.string(), timeout);
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

	preRun();
	std::ofstream file(m_path, std::ios::app);
	auto startTime = chrono::steady_clock::now();
	m_thread = std::thread(&Measurer::measure, this);
	while (true) {
		if (xtd::console::key_available()) {
			auto key = xtd::console::read_key();
			if (key.modifiers() == xtd::console_modifiers::control && key.key() == xtd::console_key::q)
				return;
		}

		if (chrono::steady_clock::now() - startTime > m_timeout) {
			startTime = chrono::steady_clock::now();
			if (m_thread.joinable())
				m_thread.join();
			file << m_meas << std::endl;
			m_thread = std::thread(&Measurer::measure, this);
		}
	}
}

void Measurer::createFile() {
	auto header = std::accumulate(
		m_meters.cbegin(),
		m_meters.cend(),
		std::string("time,s"),
		[](const std::string& prev, const std::unique_ptr<Meter>& meter) {
			return std::format("{}\t{}", prev, meter->name());
		}
	);
	std::ofstream(m_path, std::ios::out) << header << std::endl;
}

void Measurer::preRun() {
	createFile();

	xtd::console::foreground_color(xtd::console_color::cyan);
	xtd::console::write_line("Measurements started at {}", chrono::system_clock::now());
	xtd::console::write_line("Press CTRL+Q to stop measures");
	xtd::console::reset_color();

	m_start = chrono::steady_clock::now();
}

void Measurer::measure() {
	std::vector<std::thread> threads;
	std::mutex mu;
	std::unordered_map<std::string, std::string> values;

	auto cur = chrono::steady_clock::now();
	for (const auto& meter : m_meters) {
		threads.emplace_back([&meter, &values, this]() {
			meter->readFor(m_timeout);
			values[meter->name()] = meter->get();
		});
	}
	for (auto& th : threads)
		th.join();

	std::vector<std::string> res;
	for (const auto& meter : m_meters)
		res.push_back(values[meter->name()]);
	
	m_meas = Measurement{ cur - m_start, res };
}