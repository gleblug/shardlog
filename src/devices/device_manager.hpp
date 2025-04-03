#pragma once

#include <memory>
#include <vector>
#include <string>
#include <chrono>

#include "devices/device.hpp"
#include "events/data_event.hpp"

namespace chrono = std::chrono;

class DeviceManager {
public:
	explicit DeviceManager(DataBus dataBus);
    ~DeviceManager();

	void configure(const std::string& experimentName, const std::string& measurementName);

private:
	DataBus dataBus_;

	chrono::milliseconds duration_;
	chrono::milliseconds timeout_;
	chrono::milliseconds statusTimeout_;
	std::vector<std::shared_ptr<Device>> devices_;
};
