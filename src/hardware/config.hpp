#pragma once
#include <better-enums/enum.h>

#include <string>
#include <unordered_map>

BETTER_ENUM(MeterType, uint8_t,
	UNKNOWN = 1,
	VOLT_DC,
	VOLT_AC,
	CURR_DC,
	CURR_AC
)

namespace Hard {
	struct Config {
		std::string port;
		MeterType type = MeterType::UNKNOWN;
	};
	using Name = std::string;
	using ConfigMap = std::unordered_map<Name, Config>;
};
