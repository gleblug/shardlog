#pragma once

#include <unordered_map>
#include <memory>

#include "devices/device.hpp"
#include "events/data_event.hpp"

class DeviceManager {
public:
	explicit DeviceManager(DataBus dataBus);
    ~DeviceManager();

	void configure(const std::string& experimentName, const std::string& measurementName);

private:
	DataBus dataBus_;

	double duration_;
	double timeout_;
	double statusTimeout_;
	std::vector<std::shared_ptr<Device>> devices_;
};
