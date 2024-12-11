#pragma once

#include <filesystem>
#include <fstream>

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

#include "../meter/meter.hpp"

namespace lg = spdlog;
namespace fs = std::filesystem;

namespace ParserError {
	std::exception not_a_map = std::runtime_error("YAML: this is not a map!");
	std::exception not_a_sequence = std::runtime_error("YAML: this is not a sequence!");
};

void operator>>(const YAML::Node& node, Meter::Commands::List& list) {
	if (!node.IsSequence())
		throw ParserError::not_a_sequence;
	std::transform(
		node.begin(),
		node.end(),
		std::back_inserter(list),
		[](const YAML::Node& node) { return node.as<std::string>(); }
	);
}

void operator>>(const YAML::Node& node, std::vector<Meter::Commands::NamedList>& namedLists) {
	if (!node.IsMap())
		throw ParserError::not_a_map;
	auto map = node.as<std::unordered_map<std::string, std::vector<std::string>>>();
	for (const auto& [key, val] : map) {
		namedLists.emplace_back(key, val);
	}
}

void operator>>(const YAML::Node& node, Meter::Commands& commands) {
	if (!node.IsMap())
		throw ParserError::not_a_map;
	commands.name = node["name"].as<std::string>();
	if (node["configure"].IsDefined())
		node["configure"] >> commands.conf;
	node["read"] >> commands.read;
	if (node["set"].IsDefined())
		node["set"] >> commands.set;
	if (node["end"].IsDefined())
		node["end"] >> commands.end;
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
		return m_commandsMap[commandName];
	}
};