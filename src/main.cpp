#include <memory>

#include <xtd/console.h>
#include <xtd/using.h>
#include <spdlog/spdlog.h>
#include <mini/ini.h>
#include <better-enums/enum.h>

#include "config/config_parser.hpp"
#include "measurer/measurer.hpp"
#include "meter/meter.hpp"
#include "meter/connection/connection.hpp"

namespace lg = spdlog;
using xtd::console;
using xtd::ustring;

BETTER_ENUM(Option, uint8_t,
	DEVICES_LIST = 1,
	MEASURE
)

class Application {
public:
	void devicesList() {
		Visa::ResourceManager rm{};
		for (const auto& desc : rm.resourcesList()) {
			auto conn = Connection::open<Nivisa>(desc);
			console::write_line("{}\t{}", desc, conn->query("*IDN?"));
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

		lg::info("The measures ended correctly.");
	}

	void run() {
		console::clear();
		console::write_line("Please, choose option:");
		for (const auto& value : Option::_values())
			console::write_line("{} -- {}", value._to_index(), value._to_string());
		auto optionIndex = ustring::parse<size_t>(console::read_line());
		auto option = Option::_from_index(optionIndex);

		switch (option) {
		case Option::DEVICES_LIST:
			devicesList();
			break;
		case Option::MEASURE:
			measure();
			break;
		default:
			console::write_line("Unknown option!");
			break;
		}

		console::write_line("Press any button to exit...");
		console::read_key();
	}
};


auto main() -> int {
	lg::set_level(lg::level::debug);
	Application app{};
	app.run();
}