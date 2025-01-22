#include <spdlog/spdlog.h>
#include <xtd/console.h>

#include "application.hpp"

namespace lg = spdlog;
using xtd::console;

auto main() -> int {
	Application app{};
#if DEBUG
	lg::set_level(lg::level::debug);
	app.run();
#else
	lg::set_level(lg::level::info);
	try {
		app.run();
	}
	catch (const std::exception& e) {
		lg::critical("Unhandled exception: {}", e.what());
		console::read_key();
	}
#endif

}