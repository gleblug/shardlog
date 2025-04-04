#pragma once

#include <chrono>
#include <memory>

#include "event_bus/event_bus.hpp"
#include "devices/device.hpp"

struct DataEvent {
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point timestamp;
    std::map<std::string, MeasurementResult> results;
};

using DataBus = std::shared_ptr<EventBus<DataEvent>>;
