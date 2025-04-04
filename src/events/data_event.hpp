#pragma once

#include <chrono>
#include <memory>

#include "event_bus/event_bus.hpp"
#include "devices/device.hpp"

using DevicesResult = std::map<std::string, MeasurementResult>;
using DataTimepoint = std::chrono::steady_clock::time_point;

struct DataEvent {
    DataTimepoint start;
    DataTimepoint timestamp;
    DevicesResult results;
};

using DataBus = std::shared_ptr<EventBus<DataEvent>>;
