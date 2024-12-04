#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <string>

#include "../meter/meter.hpp"

namespace chrono = std::chrono;
namespace fs = std::filesystem;

struct Measurement {
	chrono::duration<double, std::ratio<1>> time;
	std::vector<std::string> values;
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
	void start();

private:
	void createFile();
	void preRun();
	Measurement processMeters();
};