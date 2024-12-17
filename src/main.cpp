#include <spdlog/spdlog.h>
#include <xtd/console.h>

#include "application.hpp"

namespace lg = spdlog;
using xtd::console;

auto main() -> int {
#if DEBUG
	lg::set_level(lg::level::debug);
#else
	lg::set_level(lg::level::warn);
#endif
	Application app{};
	try {
		app.run();
	}
	catch (const std::exception& e) {
		lg::critical("Unhandled exception: {}", e.what());
		console::read_key();
	}
}