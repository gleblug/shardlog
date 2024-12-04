#pragma once
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>
#include <string>

#include "connection/connection.hpp"

namespace lg = spdlog;

class Meter {
public:
	struct Commands {
		std::string name;
		std::vector<std::string> configure;
		std::vector<std::string> read;
	};

	struct Config {
		std::string name;
		Commands commands;
		ConnectionType connectionType;
		std::string port;
	};

	using List = std::vector<std::unique_ptr<Meter>>;

private:
	std::string m_name;
	Commands m_cmd;
	std::unique_ptr<Connection> m_conn;

public:
	Meter(const std::string& name, const Commands& cmd, std::unique_ptr<Connection>&& conn)
		: m_name{ name }
		, m_cmd{ cmd }
		, m_conn{ std::move(conn) }
	{
		for (const auto& confCmd : m_cmd.configure)
			m_conn->write(confCmd);
	}
	
	std::string name() const {
		return m_name;
	}
	
	std::string instantValue() {
		lg::debug("Get value from {}... Write commands:", m_name);
		for (const auto& readCmd : m_cmd.read) {
			m_conn->write(readCmd);
			lg::debug(readCmd);
		}
		return m_conn->read();
	}

	static std::unique_ptr<Meter> create(const Config& conf) {
		std::unique_ptr<Connection> conn;
		switch (conf.connectionType) {
		case ConnectionType::NIVISA:
			conn = Connection::open<Nivisa>(conf.port);
			break;
		case ConnectionType::COM:
			conn = Connection::open<Comport>(conf.port);
			break;
		default:
			throw std::runtime_error("Unknown connection type!");
		}
		return std::make_unique<Meter>(
			conf.name,
			conf.commands,
			std::move(conn)
		);
	}
};