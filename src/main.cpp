#include <xtd/xtd>
#include <spdlog/spdlog.h>
#include <mini/ini.h>

#include <exception>
#include <vector>
#include <string>

#include "config_parser.hpp"
#include "measurer.hpp"
#include "hardware/utils.hpp"


using namespace xtd;
namespace lg = spdlog;
namespace ini = mINI;

auto main() -> int	{
	xtd::console::title("shardlog application");

	const auto configPath = "config.ini";
	ConfigParser config(configPath);
	
	auto meters = Hard::openMeters(config.hardware());

	auto measConfig = config.measurer();
	Measurer measurer(std::move(meters), measConfig.directory, measConfig.timeout);
	measurer.run();

	ResourceManager::close();
	lg::info("The program ended correctly.");
}