#include "config_parser.hpp"

#include <xtd/io/file.h>
#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

#include <exception>

namespace io = xtd::io;
namespace lg = spdlog;

ConfigParser::ConfigParser(const std::string& filename)
	: fname{ filename }
{
	lg::info("load config from '{}'", filename);
	if (!io::file::exists(fname)) {
		lg::warn("Can't find {}. Creating a new one...", fname);
		createFile();
	}
}

void ConfigParser::createFile() {
	io::file::create(fname);

	(*this)[measurerSection].set({
		{"timeout", "0"},
		{"directory", "./data"}
		});
	save();

	io::file::append_text(fname) << "\n"
		"; meters = <DEVICE_NAME1> <DEVICE_NAME2> ...\n\n"
		"; Add hardware options like this:\n"
		"; [hardware.<DEVICE_NAME1>]\n"
		"; port = auto | specific_port\n"
		"; type = VOLT_DC | VOLT_AC | CURR_DC | CURR_AC";
}

Meas::Config ConfigParser::measurer() {
	parse();
	if (!has(measurerSection))
		throw std::runtime_error("There is no measurer section in config file!");
	auto section = get(measurerSection);
	auto timeout = xtd::ustring::parse<double>(section.get("timeout"));
	auto directory = section.get("directory");
	auto usedMeters = xtd::ustring(section.get("meters")).split();
	return { timeout, directory, usedMeters };
}

Hard::ConfigMap ConfigParser::hardware() {
	auto measConfig = measurer();
	Hard::ConfigMap configMap{};
	for (const auto& meterName : measConfig.usedMeters) {
		auto sname = xtd::ustring::format("{}.{}", hardwareSection, meterName);
		if (!this->has(sname))
			throw std::runtime_error("Invalid meter name in measurer config!");
		auto section = (*this).get(sname);

		auto port = section.get("port");
		auto type = MeterType::_from_string(section.get("type").c_str());
		configMap[meterName] = Hard::Config{ port, type };
	}
	return configMap;
}


void ConfigParser::parse() {
	ini::INIFile(fname).read(*this);
}

void ConfigParser::save() {
	ini::INIFile(fname).write(*this, true);
}