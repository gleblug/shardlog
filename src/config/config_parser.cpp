#include "config_parser.hpp"

#include <exception>
#include <fstream>

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

#include "../utils/utils.hpp"

namespace lg = spdlog;
using xtd::ustring;

ConfigParser::ConfigParser(const std::string& configPath)
	: m_path{ configPath }
{
	lg::info("Load config from '{}'...", m_path.string());
	if (!fs::exists(m_path)) {
		lg::warn("Can't find '{}'. Creating a new one from template...", m_path.string());
		createFile();
	}
	parse();
}

void ConfigParser::createFile() {
	lg::debug("Create '{}' config file...", m_path.string());
	std::ofstream(m_path, std::ios::out);

	(*this)[measurerSection].set({
		{"timeout", "0"},
		{"directory", "./data"}
		});
	save();
	// TODO : create valid example
}

Measurer::Config ConfigParser::measurer() {
	if (!has(measurerSection))
		throw std::runtime_error("There is no measurer section in config file!");
	auto section = get(measurerSection);
	return {
		utils::split(section.get("meters")),
		section.get("directory"),
		Measurer::TimeDuration(ustring::parse<double>(section.get("duration"))),
		Measurer::TimeDuration(ustring::parse<double>(section.get("timeout"))),
	};
}

Meter::Config ConfigParser::meter(const std::string &name) {
	auto sname = std::format("{}.{}", meterSection, name);
	if (!has(sname))
		throw std::runtime_error(std::format("Invalid meter name '{}' in measurer config!", sname));
	auto section = get(sname);

	return {
		section.get("commands"),
		section.get("port"),
		section.get("setValues")
	};
}

void ConfigParser::parse() {
	lg::debug("Parsing '{}' config file...", m_path.string());
	ini::INIFile(m_path.string()).read(*this);
}

void ConfigParser::save() {
	lg::debug("Saving '{}' config file...", m_path.string());
	ini::INIFile(m_path.string()).write(*this, true);
}