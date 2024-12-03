#pragma once

#include "visa_instrument.hpp"

namespace Hard {
	Meter::List openMeters(const ConfigMap& configs) {
		auto rm = std::make_unique<ResourceManager>();

		std::vector<std::unique_ptr<Meter>> meters{};
		for (const auto& [name, conf] : configs) {
			lg::info("load config = '{}',\tport = '{}'\ttype = '{}'", name, conf.port, conf.type._to_string());
			if (conf.type == +MeterType::UNKNOWN) {
				lg::error("unknown meter type");
				continue;
			}
			meters.push_back(std::make_unique<VisaInstrument>(
				rm->openResource(conf.port),
				name,
				conf.type
			));
		}

		return meters;
	}
};