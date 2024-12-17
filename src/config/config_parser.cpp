#include "config_parser.hpp"

#include <exception>
#include <fstream>
#include <filesystem>

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;
namespace fs = std::filesystem;

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

Meter::Config ConfigParser::meter(const std::string &name) {
	auto sname = std::format("{}.{}", meterSection, name);
	if (!has(sname))
		throw std::runtime_error(std::format("Invalid meter name '{}' in measurer config!", sname));
	auto section = get(sname);

	auto setDataPath = fs::path(section.get("setData"));
	std::vector<Meter::SetValue> setData;
	if (!setDataPath.empty()) {
		std::ifstream setDataFile(setDataPath);
		std::string line;
		while (std::getline(setDataFile, line)) {
			auto splitLine = xtd::ustring(line).split({ ',', '\t' });
			std::transform(splitLine.cbegin(), splitLine.cend(), splitLine.begin(), [](const xtd::ustring& s) { return s.trim(); });
			auto time = chrono::duration<double>(xtd::ustring::parse<double>(splitLine.at(0)));
			std::vector<std::string> args(std::next(splitLine.cbegin()), splitLine.cend());
			setData.emplace_back(time, args);
		}
	}

	return Meter::Config{
		name,
		section.get("commands"),
		section.get("port"),
		setData
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