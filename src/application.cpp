#include "application.hpp"

#include <spdlog/spdlog.h>
namespace lg = spdlog;

Application::Application()
	: dataBus_{std::make_shared<EventBus<DataEvent>>()}
	, commandBus_{std::make_shared<EventBus<CommandEvent>>()}
	, connectionBus_{std::make_shared<EventBus<ConnectionEvent>>()}
	, frontend_(commandBus_)
	, connectionManager_(connectionBus_)
{}

void Application::run() {
	lg::info("Application started");

	commandBus_->subscribe("app_command_handler", [this](const CommandEvent& event) {
		handleCommand(event);
	});
	connectionBus_->subscribe("frontend_connection_handler", [this](const ConnectionEvent& event) {
		frontend_.handleConnection(event);
	});
	dataBus_->subscribe("receivers_data_handler", [this](const DataEvent& event) {
		for (auto& [name, receiver_ptr] : receivers_) {
			receiver_ptr->receive(event);
		}
	});

	connectionManager_.start();
	frontend_.run();

	dataBus_->unsubscribe("receivers_data_handler");
	connectionBus_->unsubscribe("frontend_connection_handler");
	commandBus_->unsubscribe("app_command_handler");

	lg::info("Application stopped");
}

void Application::handleCommand(const CommandEvent& event) {
	switch(event.type) {
	case CommandEvent::Type::START:
		break;
	case CommandEvent::Type::STOP:
		break;
	case CommandEvent::Type::SET:
		if (!event.parameters.contains("name")) {
			lg::error("There is no name to set measurement");
			break;
		}
		setMeasurement(event.parameters.at("name"));
		break;
	default:
		lg::warn("Unknown command on app handler");
		break;
	}
}

void Application::setMeasurement(const std::string& name) {
	
}
