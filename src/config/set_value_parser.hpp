#pragma once

#include <string>
#include <filesystem>

#include "../utils/string.hpp"
#include "../meter/meter.hpp"

namespace fs = std::filesystem;

class SetValueParser {
private:
	fs::path m_path;
	std::vector<Meter::SetValue> m_values;

public:
	SetValueParser(const std::string& path)
		: m_path(path)
		, m_values{}
	{
		if (m_path.empty())
			return;

		std::ifstream setDataFile(m_path);
		std::string line;
		while (std::getline(setDataFile, line)) {
			auto splitLine = utils::split(line);
			auto time = chrono::duration<double>(ustring::parse<double>(splitLine.at(0)));
			std::vector<std::string> args(std::next(splitLine.cbegin()), splitLine.cend());
			m_values.emplace_back(time, args);
		}
	}
	std::vector<Meter::SetValue> get() const {
		return m_values;
	}
};