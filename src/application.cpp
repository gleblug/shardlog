#include "application.hpp"

#include <xtd/console.h>
#include <spdlog/spdlog.h>
#include <CSerialPort/SerialPortInfo.h>

#include "config/config_parser.hpp"
#include "config/command_parser.hpp"
#include "measurer/measurer.hpp"
#include "meter/meter.hpp"
#include "meter/connection/connection.hpp"
#include "ui/console_choose.hpp"

namespace lg = spdlog;
using xtd::console;

Application::Application() {
	console::cursor_visible(false);
};

void Application::run() {
	ConsoleChoose::show({
		{"List of devices", [this]() { devicesList(); }},
		{"Start measurements", [this]() { measurements(); }}
	});
}

void Application::devicesList() {
	Visa::ResourceManager rm{};
	console::write_line("VISA devices:");
	for (const auto& desc : rm.resourcesList()) {
		auto conn = Connection::open<Nivisa>(desc);
		console::write_line("{}\t{}", desc, conn->query("*IDN?"));
	}
	console::write_line("COM devices:");
	for (const auto& comport : CSerialPortInfo::availablePortInfos()) {
		auto conn = Connection::open<Comport>(comport.portName);
		console::write_line("{}\t{}\t{}\t{}", comport.portName, comport.description, conn->query("*IDN?"));
	}
}

void Application::measurements() {
	const auto configPath = "config.ini";
	ConfigParser config(configPath);
	auto measConfig = config.measurer();
	
	const auto commandsPath = "commands.yaml";
	CommandParser commands(commandsPath);

	Meter::List meters;
	for (const auto& name : measConfig.usedMeters) {
		auto meterConfig = config.meter(name);
		meters.push_back(std::make_unique<Meter>(
			meterConfig.name,
			meterConfig.port,
			meterConfig.setData,
			commands.get(meterConfig.commandsName)
		));
	}

	Measurer measurer(std::move(meters), measConfig.directory, measConfig.timeout);
	measurer.start();

	lg::info("The measures ends correctly.");
}

