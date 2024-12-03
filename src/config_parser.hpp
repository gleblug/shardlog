#pragma once

#include <mini/ini.h>
#include <xtd/ustring.h>

#include "hardware/meter.hpp"
#include "hardware/config.hpp"

namespace ini = mINI;

namespace Meas {
	struct Config {
		double timeout;
		std::string directory;
		std::vector<xtd::ustring> usedMeters;
	};
};

class ConfigParser : private ini::INIStructure {

	std::string fname;
	const std::string measurerSection = "measurer";
	const std::string hardwareSection = "hardware";

public:
	explicit ConfigParser(const std::string& filename);
	void createFile();
	Meas::Config measurer();
	Hard::ConfigMap hardware();

private:
	void parse();
	void save();
};