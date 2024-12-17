#pragma once

#include <filesystem>

#include <mini/ini.h>
#include <xtd/ustring.h>

#include "../meter/meter.hpp"
#include "../measurer/measurer.hpp"

namespace ini = mINI;
namespace fs = std::filesystem;

class ConfigParser : private ini::INIStructure {
private:
	fs::path m_path;
	const std::string measurerSection = "measurer";
	const std::string meterSection = "meter";

public:
	explicit ConfigParser(const std::string& configPath);
	Measurer::Config measurer();
	Meter::Config meter(const std::string& name);

private:
	void createFile();
	void parse();
	void save();
};