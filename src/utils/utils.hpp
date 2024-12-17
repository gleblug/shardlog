#pragma once

#include <vector>
#include <string>

#include <xtd/ustring.h>

using xtd::ustring;

namespace utils {
	const inline std::vector<char> separators = { ' ', ',', '\t' };

	inline std::vector<std::string> split(const std::string& str) {
		auto vec = ustring(str).split(separators);
		std::vector<std::string> res;
		std::transform(vec.cbegin(), vec.cend(), std::back_inserter(res), [](const ustring& us) { return us.trim(); });
		return res;
	}
}