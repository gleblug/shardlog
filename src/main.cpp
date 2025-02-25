#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "hardware/serial/ListSerial.hpp"

namespace lg = spdlog;

auto main() -> int {
	lg::set_default_logger(lg::basic_logger_mt("basic_logger", "logs/shardlog.log"));
	lg::set_level(lg::level::debug);
	lg::debug("Hello, shardlog!");
	lg::debug("Ports: {}", ListSerial::all_ports().at(0));
}
