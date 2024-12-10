#include <spdlog/spdlog.h>

#include "application.hpp"

namespace lg = spdlog;

auto main() -> int {
	lg::set_level(lg::level::info);
	Application app{};
	try {
		app.run();
	}
	catch (const std::exception& e) {
		lg::critical("Unhandled exception: {}", e.what());
	}
}