#include "measurer.hpp"

#include <thread>
#include <format>
#include <numeric>
#include <unordered_map>
#include <fstream>

#include <xtd/console.h>
#include <xtd/ustring.h>
#include <fmt/format.h>

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
	std::ofstream(m_path, std::ios::out);
	lg::info("Save data into '{}' with timeout '{}s'", m_path.string(), m_timeout.count());
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

	xtd::console::foreground_color(xtd::console_color::cyan);
	xtd::console::write_line("Measurements started at {}", chrono::system_clock::now());
	xtd::console::write_line("Press CTRL+Q to stop measures");

	loop(file);

	xtd::console::write_line("Average response time:");
	for (const auto& meter : m_meters) {
		xtd::console::write_line(fmt::format(
			"{:<16}{}ms",
			meter->name(),
			chrono::duration_cast<chrono::milliseconds>(meter->averageResponseTime()).count()
		));
	}
	xtd::console::reset_color();
}

std::string Measurer::header() {
	return std::accumulate(
		m_meters.cbegin(),
		m_meters.cend(),
		std::string("time,s"),
		[](const std::string& prev, const std::unique_ptr<Meter>& meter) {
			return std::format("{}\t{}", prev, meter->name());
		}
	);
	
}

void Measurer::measure() {
	std::vector<std::thread> threads;
	std::mutex mu;
	std::unordered_map<std::string, std::string> values;
	
	auto curTime = chrono::steady_clock::now();

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
	std::lock_guard lg(m_mu);
	m_meas = Measurement{ curTime - m_start, res };
}

void Measurer::setMetersData() {
	std::vector<std::thread> threads;
	for (const auto& meter : m_meters) {
		if (
			meter->needToSet() &&
			meter->currentSetValue().time <= (chrono::steady_clock::now() - m_start)
		) {
			threads.emplace_back([&meter]() { meter->setCurrentData(); });
		}
	}
	for (auto& th : threads)
		th.join();
}

void Measurer::loop(std::ostream& os) {
	m_start = chrono::steady_clock::now();
	auto startLastTime = m_start;

	setMetersData();
	m_thread = std::thread(&Measurer::measure, this);
	while (true) {
		if (xtd::console::key_available()) {
			auto key = xtd::console::read_key();
			if (key.modifiers() == xtd::console_modifiers::control && key.key() == xtd::console_key::q) {
				xtd::console::write_line();
				break;
			}
		}

		if (chrono::steady_clock::now() - startLastTime > m_timeout) {
			startLastTime = chrono::steady_clock::now();
			m_thread.join();
			std::lock_guard lg(m_mu);
			setMetersData();
			m_thread = std::thread(&Measurer::measure, this);
			os << m_meas << std::endl;
		}
	}
}