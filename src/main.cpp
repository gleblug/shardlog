#include <spdlog/spdlog.h>
#include <xtd/console.h>

#include "application.hpp"

namespace lg = spdlog;
using xtd::console;

auto main() -> int {
	lg::set_level(lg::level::debug);
	Application app{};
	app.run();
	//try {
	//	app.run();
	//}
	//catch (const std::exception& e) {
	//	lg::critical("Unhandled exception: {}", e.what());
	//	console::read_key();
	//}
}