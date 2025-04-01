#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "events/data_event.hpp"
#include "events/command_event.hpp"
#include "events/connection_event.hpp"
#include "event_bus/event_bus.hpp"
#include "devices/device_manager.hpp"
#include "receivers/receiver.hpp"
#include "frontend/console_frontend.hpp"
#include "connection/connection_manager.hpp"

class Application {
public:
	Application();
	void run();
private:
	void handleCommand(const CommandEvent& event);
	
	void setMeasurement(const std::string& name);
	// void startMeasurement();
	// void stopMeasurement();

	DataBus dataBus_;
	CommandBus commandBus_;
	ConnectionBus connectionBus_;

	DeviceManager deviceManager_;
	std::unordered_map<std::string, std::shared_ptr<IReceiver>> receivers_;
	ConnectionManager connectionManager_;

	ConsoleFrontend frontend_;
};
