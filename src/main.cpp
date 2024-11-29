#include <xtd/xtd>
#include <spdlog/spdlog.h>

using namespace xtd;

auto main() -> int {
	console::foreground_color(console_color::white);
	console::write_line("Hello, World!");
	spdlog::info("INFO");
}