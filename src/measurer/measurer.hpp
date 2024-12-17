#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <queue>
#include <cstdint>

#include "../meter/meter.hpp"

namespace chrono = std::chrono;
namespace fs = std::filesystem;

struct Measurement {
	chrono::duration<double> fromStart;
	std::vector<Meter::Value> values;
};

std::ostream& operator<<(std::ostream& os, const Measurement& meas);

class Measurer {
public:
	using TimePoint = chrono::steady_clock::time_point;
	using TimeDuration = chrono::duration<double, std::ratio<1>>;

	struct Config {
		std::vector<Meter::Name> meterNames;
		fs::path directory;
		TimeDuration duration;
		TimeDuration timeout;
	};

private:
	std::vector<Meter::Ptr> m_meters;
	fs::path m_path;
	TimeDuration m_duration;
	TimeDuration m_timeout;
	uint64_t m_counter;

	TimePoint m_start;
	Measurement m_meas;
	std::thread m_thread;
	std::mutex m_mu;

public:
	Measurer(std::vector<Meter::Ptr>&& meters, const fs::path& directory, const TimeDuration& duration, const TimeDuration& timeout);
	~Measurer();
	void start();

private:
	std::string header() const;
	void measure();
	void setMetersData();
	void loop(std::ostream& os);
};