#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "application.hpp"

namespace lg = spdlog;

int main(int argc, char* argv[]) {
	const auto log_name = PROJECT_NAME;
	const auto log_file = "logs/shardlog.log";
	#if DEBUG
	const auto log_level = lg::level::debug;
	#else
	const auto log_level = lg::level::info;
	#endif

	try {
		lg::set_default_logger(lg::basic_logger_mt(log_name, log_file));
		lg::set_level(log_level);
		lg::flush_on(lg::level::debug);

		Application app;
		app.run();

		lg::info("Application finished correctly");

	} catch(const std::exception& e) {
		lg::critical(std::string("Exception: ") + e.what());
		return EXIT_FAILURE;
	} catch(...) {
		lg::critical("Unknown exception");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
