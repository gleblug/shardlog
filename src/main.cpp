#include <xtd/xtd>
#include <spdlog/spdlog.h>

using namespace xtd;

auto main() -> int	{
	console::foreground_color(console_color::white);

	spdlog::error("cannot connect to visa");

	spdlog::info("INFO");
}