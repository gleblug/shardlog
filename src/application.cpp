#include "application.hpp"

#include <spdlog/spdlog.h>
namespace lg = spdlog;

Application::Application() {}

void Application::run() {
	lg::info("Application started");

	commandBus_->subscribe("app_command_handler", [this](const CommandEvent& event) {
		handleCommand(event);
	});

	frontend_.run();

	stopAllDevices();

	commandBus_->unsubscribe("app_command_handler");

	lg::info("Application stopped");
}

void Application::handleCommand(const CommandEvent& event) {
	
}

void Application::startAllDevices() {
    for (auto& [name, device_ptr] : devices_) {
        device_ptr->start();
		lg::info("Started device: " + name);
    }
}

void Application::stopAllDevices() {
    for (auto & [name, device_ptr] : devices_) {
        device_ptr->stop();
        lg::info("Stopped device: " + name);
    }
}
