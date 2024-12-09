#pragma once

#include <vector>
#include <functional>
#include <string>

#include <xtd/console.h>

class ConsoleChoose {
public:
	static void show(std::vector<std::pair<std::string, std::function<void()>>> handlers) {
		std::vector<std::string> items;
		std::transform(handlers.cbegin(), handlers.cend(), std::back_inserter(items), [](const std::pair<std::string, std::function<void()>>& p) {
			return p.first;
		});
		
		size_t curIdx = 0;
		updateScreen(items, curIdx);
		while (true) {
			if (xtd::console::key_available()) {
				switch (xtd::console::read_key().key()) {
				case xtd::console_key::q:
					return;
				case xtd::console_key::up_arrow:
					(curIdx > 0) ? (--curIdx) : 0;
					break;
				case xtd::console_key::down_arrow:
					(curIdx < items.size() - 1) ? (++curIdx) : 0;
					break;
				case xtd::console_key::enter:
					xtd::console::clear();
					handlers.at(curIdx).second();
					xtd::console::write_line("Press any key to return.");
					xtd::console::read_key();
					break;
				default:
					break;
				}
				updateScreen(items, curIdx);
			}
		}
	}

private:
	static void updateScreen(std::vector<std::string> items, const size_t idx) {
		xtd::console::clear();
		for (size_t i = 0; i < items.size(); ++i) {
			auto item = items.at(i);
			if (i == idx) {
				xtd::console::write_line("-> {}", item);
				continue;
			}
			xtd::console::write_line("   {}", item);
		}
		xtd::console::write_line("\nPress 'q' to exit.");
	}
};