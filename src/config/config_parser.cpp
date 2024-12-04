#include "config_parser.hpp"

#include <exception>
#include <fstream>

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

#include "command_parser.hpp"

namespace lg = spdlog;

namespace Utils {
	std::vector<std::string> split(const std::string& str, const char delim) {
		std::vector<std::string> res{};
		std::istringstream iss(str);
		std::string buffer;
		while (std::getline(iss, buffer, delim))
			res.push_back(buffer);
		return res;
	}
};

ConfigParser::ConfigParser(const std::string& configPath)
	: m_path{ configPath }
{
	lg::info("Load config from '{}'...", m_path.string());
	if (!fs::exists(m_path)) {
		lg::warn("Can't find '{}'. Creating a new one from template...", m_path.string());
		createFile();
	}
}

void ConfigParser::createFile() {
	lg::debug("Create '{}' config file...", m_path.string());
	std::ofstream(m_path, std::ios::out);

	(*this)[measurerSection].set({
		{"timeout", "0"},
		{"directory", "./data"}
		});
	save();

	std::ofstream(m_path, std::ios::app) << "\n"
		"; meters = <METER_NAME1> <METER_NAME2> ...\n\n"
		"; Add meter options like this:\n"
		"; [meter.<METER_NAME1>]\n"
		"; commands = keysight1234 | rigol5678 -- from commands.yaml\n"
		"; connection = com | visa\n"
		"; port = auto | specific_port\n";
}

ConfigParser::MeasurerConfig ConfigParser::measurer() {
	parse();
	if (!has(measurerSection))
		throw std::runtime_error("There is no measurer section in config file!");
	auto section = get(measurerSection);

	return {
		xtd::ustring::parse<double>(section.get("timeout")),
		section.get("directory"),
		Utils::split(section.get("meters"), ' ')
	};
}

std::vector<Meter::Config> ConfigParser::meters() {
	auto measConfig = measurer();
	CommandParser commands("commands.yaml");
	
	std::vector<Meter::Config> res;
	for (const auto& meterName : measConfig.usedMeters) {
		auto sname = std::format("{}.{}", meterSection, meterName);
		if (!has(sname))
			throw std::runtime_error("Invalid meter name in measurer config!");
		auto section = get(sname);

		res.emplace_back(
			meterName,
			commands.get(section.get("commands")),
			ConnectionType::_from_string(section.get("connection").c_str()),
			section.get("port")
		);
	}
	return res;
}


void ConfigParser::parse() {
	lg::debug("Parsing '{}' config file...", m_path.string());
	ini::INIFile(m_path.string()).read(*this);
}

void ConfigParser::save() {
	lg::debug("Saving '{}' config file...", m_path.string());
	ini::INIFile(m_path.string()).write(*this, true);
}