#pragma once

#include <mini/ini.h>
#include <better-enums/enum.h>
#include <xtd/io/file.h>
#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

#include <exception>

#include "meter.hpp"

namespace io = xtd::io;
namespace ini = mINI;
namespace lg = spdlog;

class Config : private ini::INIStructure {
	using HardwareName = std::string;

	struct HardwareConfig {
		std::string port;
		MeterType type = MeterType::UNKNOWN;
	};

	struct MeasurerConfig {
		double timeout;
		std::string directory;
		std::vector<xtd::ustring> usedMeters;
	};

	std::string fname;

	const std::string measurerSection = "measurer";
	const std::string hardwareSection = "hardware";

public:
	explicit Config(const std::string& filename) noexcept : fname{ filename } {
		if (!io::file::exists(fname)) {
			lg::warn("Can't find {}. Creating a new one...", fname);
			createFile();
		}
	}

	void createFile() {
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

	auto measurer() {
		parse();
		if (!this->has(measurerSection))
			throw std::runtime_error("There is no measurer section in config file!");
		auto section = (*this).get(measurerSection);
		auto timeout = xtd::ustring::parse<double>(section.get("timeout"));
		auto directory = section.get("directory");
		auto usedMeters = xtd::ustring(section.get("meters")).split();
		return MeasurerConfig{timeout, directory, usedMeters};
	}

	auto hardware() {
		auto measConfig = measurer();
		std::unordered_map<HardwareName, HardwareConfig> hardConfig{};
		for (const auto& meterName : measConfig.usedMeters) {
			auto sname = xtd::ustring::format("{}.{}", hardwareSection, meterName);
			if (!this->has(sname))
				throw std::runtime_error("Invalid meter name in measurer config!");
			auto section = (*this).get(sname);

			auto port = section.get("port");
			auto type = MeterType::_from_string(section.get("type").c_str());
			hardConfig[meterName] = HardwareConfig{ port, type };
		}
		return hardConfig;
	}

private:
	void parse() {
		ini::INIFile(fname).read(*this);
	}

	void save() {
		ini::INIFile(fname).write(*this, true);
	}
};