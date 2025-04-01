#pragma once

#include <chrono>
#include <memory>

#include "event_bus/event_bus.hpp"

struct DataEvent {
    std::string device_port;
    std::string device_name;
    std::chrono::steady_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> values;
};

using DataBus = std::shared_ptr<EventBus<DataEvent>>;
