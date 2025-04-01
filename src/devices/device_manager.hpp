#pragma once

#include <unordered_map>
#include <memory>

#include "devices/device.hpp"

class DeviceManager {
public:
	explicit DeviceManager(DataBus dataBus){}
    ~DeviceManager(){}

private:
	std::unordered_map<std::string, std::shared_ptr<IDevice>> devices_;
};
