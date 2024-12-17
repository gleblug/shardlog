#pragma once

#include <filesystem>

#include <mini/ini.h>
#include <xtd/ustring.h>
#include <better-enums/enum.h>

#include "../meter/meter.hpp"

namespace ini = mINI;
namespace fs = std::filesystem;

class ConfigParser : private ini::INIStructure {
public:
	struct MeasurerConfig {
		double timeout;
		std::string directory;
		std::vector<std::string> usedMeters;
	};

private:
	fs::path m_path;
	const std::string measurerSection = "measurer";
	const std::string meterSection = "meter";

public:
	explicit ConfigParser(const std::string& configPath);
	MeasurerConfig measurer();
	Meter::Config meter(const std::string& name);

private:
	void createFile();
	void parse();
	void save();
};