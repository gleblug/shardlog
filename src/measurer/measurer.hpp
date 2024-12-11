#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <queue>

#include "../meter/meter.hpp"

namespace chrono = std::chrono;
namespace fs = std::filesystem;

struct Measurement {
	chrono::duration<double, std::ratio<1>> time;
	std::vector<std::string> values;
};

std::ostream& operator<<(std::ostream& os, const Measurement& meas);

class Measurer {

	const Meter::List m_meters;
	fs::path m_path;
	chrono::duration<double> m_timeout;
	chrono::steady_clock::time_point m_start;
	Measurement m_meas;
	std::thread m_thread;
	std::mutex m_mu;

public:
	Measurer(Meter::List&& meters, const std::string& directory, const double timeout);
	~Measurer();
	void start();

private:
	std::string header() const;
	void measure();
	void setMetersData();
	void loop(std::ostream& os);
};