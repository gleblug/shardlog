#pragma once

#include <chrono>
#include <memory>
#include <map>
#include <string>

#include "event_bus/event_bus.hpp"

struct DeviceData {
    std::string port;
    std::vector<std::pair<std::string, std::string>> values;
};
using DataTimepoint = std::chrono::steady_clock::time_point;

struct DataEvent {
    DataTimepoint start;
    DataTimepoint timestamp;
    DataTimepoint end;
    std::map<std::string, DeviceData> results;
};

using DataBus = std::shared_ptr<EventBus<DataEvent>>;
