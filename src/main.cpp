#include <memory>

#include <xtd/console.h>
#include <xtd/using.h>
#include <spdlog/spdlog.h>
#include <mini/ini.h>
#include <better-enums/enum.h>
#include <CSerialPort/SerialPortInfo.h>

#include "config/config_parser.hpp"
#include "measurer/measurer.hpp"
#include "meter/meter.hpp"
#include "meter/connection/connection.hpp"
#include "ui/console_choose.hpp"

namespace lg = spdlog;
using xtd::console;
using xtd::ustring;

class Application {
public:
	Application() {
		xtd::console::cursor_visible(false);
	};

	void devicesList() {
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

	void measure() {
		const auto configPath = "config.ini";
		ConfigParser config(configPath);

		Meter::List meters;
		for (const auto& mconf : config.meters())
			meters.push_back(Meter::create(mconf));

		auto measConfig = config.measurer();
		Measurer measurer(std::move(meters), measConfig.directory, measConfig.timeout);
		measurer.start();

		lg::info("The measures ends correctly.");
	}

	void run() {
		ConsoleChoose::show({
			{"List of devices", [this]() { devicesList(); }},
			{"Start measurements", [this]() { measure(); }}
		});
	}
};


auto main() -> int {
	lg::set_level(lg::level::info);
	Application app{};
	app.run();
}