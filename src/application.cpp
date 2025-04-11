#include "application.hpp"

#include <spdlog/spdlog.h>
namespace lg = spdlog;

Application::Application()
	: dataBus_{std::make_shared<EventBus<DataEvent>>()}
	, connectionBus_{std::make_shared<EventBus<ConnectionEvent>>()}
	, commandBus_{std::make_shared<EventBus<CommandEvent>>()}
	, deviceManager_(dataBus_, connectionBus_)
	, connectionManager_(connectionBus_)
	, receiverManager_()
	, frontend_(commandBus_)
{}

void Application::run() {
	lg::info("Application started");

	commandBus_->subscribe("app_command_handler", [this](const CommandEvent& event) {
		handleCommand(event);
	});
	connectionBus_->subscribe("frontend_connection_handler", [this](const ConnectionEvent& event) {
		frontend_.handleConnection(event);
	});
	dataBus_->subscribe("frontend_data_handler", [this](const DataEvent& event) {
		frontend_.handleData(event);
	});
	dataBus_->subscribe("receivers_data_handler", [this](const DataEvent& event) {
		receiverManager_.handleData(event);
	});

	connectionManager_.start();
	frontend_.run();

	lg::info("Application stopped");
}

void Application::handleCommand(const CommandEvent& event) {
	switch(event.type) {
	case CommandEvent::Type::START_MEASUREMENT:
		startMeasurement();
		break;
	case CommandEvent::Type::STOP_MEASUREMENT:
		stopMeasurement();
		break;
	case CommandEvent::Type::ACTIVATE_MEASUREMENT:
		activateMeasurement(event.parameters.at("experiment_name"), event.parameters.at("measurement_name"));
		break;
	default:
		lg::warn("Unknown command on app handler");
		break;
	}
}

void Application::activateMeasurement(const std::string& experimentName, const std::string& measurementName) {
	receiverManager_.configure(experimentName, measurementName);
	deviceManager_.configure(experimentName, measurementName);
	lg::info("Activating experiment '{}', measurement '{}'", experimentName, measurementName);
}

void Application::startMeasurement() {
	receiverManager_.reset();
	deviceManager_.start();
	lg::info("Start measurement");
}

void Application::stopMeasurement() {
	deviceManager_.stop();
	lg::info("Stop measurement");
}
