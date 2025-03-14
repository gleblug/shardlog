#pragma once

#include <vector>
#include <functional>
#include <string>

#include <xtd/console.h>

using xtd::console;
using xtd::console_key;

class ConsoleChoose {
public:
	static void show(std::vector<std::pair<std::string, std::function<void()>>> handlers) {
		std::vector<std::string> items;
		for (const auto& handler : handlers)
			items.push_back(handler.first);
		items.push_back("Return");
		
		size_t curIdx = 0;
		updateScreen(items, curIdx);
		while (true) {
			if (console::key_available()) {
				switch (console::read_key().key()) {
				case console_key::up_arrow:
					(curIdx > 0) ? (--curIdx) : 0;
					break;
				case console_key::down_arrow:
					(curIdx < items.size() - 1) ? (++curIdx) : 0;
					break;
				case console_key::enter:
					if (curIdx == handlers.size())
						return;
					console::clear();
					handlers.at(curIdx).second();
					console::write_line("Press any key to return.");
					console::read_key();
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
		console::clear();
		for (size_t i = 0; i < items.size(); ++i) {
			auto item = items.at(i);
			if (i == items.size() - 1)
				console::write_line();
			if (i == idx) {
				console::write_line("-> {}", item);
				continue;
			}
			console::write_line("   {}", item);
		}
	}
};