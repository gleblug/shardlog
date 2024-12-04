#pragma once

#include <filesystem>
#include <fstream>

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

#include "../meter/meter.hpp"

namespace lg = spdlog;
namespace fs = std::filesystem;

void operator>>(const YAML::Node& node, std::vector<std::string>& strings) {
	std::transform(
		node.begin(),
		node.end(),
		std::back_inserter(strings),
		[](const YAML::Node& node) { return node.as<std::string>(); }
	);
}

void operator>>(const YAML::Node& node, Meter::Commands& commands) {
	commands.name = node["name"].as<std::string>();
	node["configure"] >> commands.configure;
	node["read"] >> commands.read;
}

class CommandParser {
	fs::path m_path;
	std::unordered_map<std::string, Meter::Commands> m_commandsMap;

public:
	CommandParser(const std::string& configPath)
		: m_path(configPath)
		, m_commandsMap{}
	{
		lg::debug("Load commands config from '{}'...", m_path.string());
		auto doc = YAML::LoadFile(m_path.string());
		for (unsigned i = 0; i < doc.size(); i++) {
			Meter::Commands commands;
			doc[i] >> commands;
			m_commandsMap.insert({ commands.name, commands });
			lg::debug("'{}' commands set loaded.", commands.name);
		}
	}

	Meter::Commands get(const std::string& commandName) {
		if (!m_commandsMap.contains(commandName))
			throw std::runtime_error(std::format("There is no {} in available commands!", commandName));
		return m_commandsMap.at(commandName);
	}
};