#pragma once
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

#include "config.hpp"

namespace lg = spdlog;

class Meter {
	Hard::Name meterName;
	MeterType meterType;
public:
	using List = std::vector<std::unique_ptr<Meter>>;

	Meter(const Hard::Name& name, const MeterType mtype) : meterName(name), meterType(mtype) {}
	virtual ~Meter() = default;
	
	std::string name() const { return meterName; }
	MeterType type() const { return meterType; }
	virtual double value() = 0;
};