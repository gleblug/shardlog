#pragma once

#include <chrono>

struct DeviceDataEvent {
    std::string device_name;
    std::chrono::steady_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> values;
};
