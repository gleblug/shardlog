#include <xtd/xtd>
#include <spdlog/spdlog.h>
#include <mini/ini.h>

#include <exception>
#include <vector>
#include <string>

#include "config.hpp"
#include "meter.hpp"
#include "visa_resources.hpp"


using namespace xtd;
namespace lg = spdlog;
namespace ini = mINI;

auto main() -> int	{
	const auto configPath = "config.ini";
	lg::info("load config from '{}'", configPath);
	Config config(configPath);
	auto measConfig = config.measurer();
	lg::info("measurer will save file into '{}' with timeout t = {}s", measConfig.directory, measConfig.timeout);
	auto hardConfigMap = config.hardware();
	lg::info("load {} devices...", hardConfigMap.size());

	lg::info("open resource manager");
	ResourceManager rm{};

	std::vector<std::unique_ptr<Meter>> meters{};
	for (const auto& [name, conf] : hardConfigMap) {
		lg::info("load config = '{}',\tport = '{}'\ttype = '{}'", name, conf.port, conf.type._to_string());
		if (conf.type == +MeterType::UNKNOWN) {
			lg::error("unknown meter type");
			continue;
		}
		meters.push_back(std::make_unique<VisaInstrument>(
			rm.openResource(conf.port),
			conf.type
		));
	}
	// create measurer instance

	// run measures

	/*for (auto& desc : rm.resourcesList()) {
		lg::info(desc);
		auto r = rm.openResource(desc);
		lg::info(r->idn);
	}*/
}