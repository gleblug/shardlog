#pragma once

#include "config.hpp"
#include "meter.hpp"

namespace Hard {
	Meter::List openMeters(const ConfigMap& configs);
};