#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "events/device_data_event.hpp"
#include "events/command_event.hpp"
#include "event_bus/event_bus.hpp"
#include "devices/device.hpp"
#include "receivers/receiver.hpp"
#include "frontend/console_frontend.hpp"

class Application {
public:
	Application();
	void run();
private:
	void handleCommand(const CommandEvent& event);
	void startAllDevices();
	void stopAllDevices();

	std::shared_ptr<EventBus<DeviceDataEvent>> dataBus_;
	std::shared_ptr<EventBus<CommandEvent>> commandBus_;

	std::unordered_map<std::string, std::shared_ptr<IDevice>> devices_;
	std::unordered_map<std::string, std::shared_ptr<IReceiver>> receivers_;

	ConsoleFrontend frontend_;
};
