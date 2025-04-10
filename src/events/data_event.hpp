#pragma once

#include <chrono>
#include <memory>
#include <map>

#include "event_bus/event_bus.hpp"

enum class DeviceStatus {
    READY,
    TIMEOUT,
    DISCONNECTED
};

struct DeviceData {
    DeviceStatus status;
    std::vector<std::pair<std::string, std::string>> values;
};

using DevicesResult = std::map<std::string, DeviceData>;
using DataTimepoint = std::chrono::steady_clock::time_point;

enum class DataType {
    STATUS,
    WORKLOAD,
    UNKNOWN
};

struct DataEvent {
    DataTimepoint start;
    DataTimepoint timestamp;
    DevicesResult results;
};

using DataBus = std::shared_ptr<EventBus<DataEvent>>;
