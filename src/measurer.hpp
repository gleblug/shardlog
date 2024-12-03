#pragma once

#include "hardware/meter.hpp"

#include <chrono>
#include <filesystem>
#include <vector>
#include <string>

namespace chrono = std::chrono;
namespace fs = std::filesystem;

struct Measurement {
	chrono::duration<double, std::ratio<1>> time;
	std::vector<double> values;
};

std::ostream& operator<<(std::ostream& os, const Measurement& meas);

class Measurer {

	const Meter::List meters;
	fs::path directory;
	fs::path fname;
	double timeout;
	chrono::steady_clock::time_point startTime;

public:
	Measurer(Meter::List&& meters_, const std::string& directory_, const double timeout_);
	void run();

private:
	void createFile();
	void preRun();
	Measurement processMeters();
};