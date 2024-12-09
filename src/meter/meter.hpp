#pragma once
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>
#include <string>
#include <chrono>

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
	std::string m_value;
	std::atomic_flag m_recite;

public:
	std::mutex m_mu;
	std::condition_variable m_cv;

	Meter(const std::string& name, const Commands& cmd, std::unique_ptr<Connection>&& conn)
		: m_name{ name }
		, m_cmd{ cmd }
		, m_conn{ std::move(conn) }
		, m_mu{}
		, m_cv{}
		, m_value{}
	{
		for (const auto& cmd : m_cmd.read)
			lg::debug("Read command: {}", cmd);
		for (const auto& cmd : m_cmd.configure)
			lg::debug("Config command: {}", cmd);
		for (const auto& confCmd : m_cmd.configure)
			m_conn->write(confCmd);
	}
	
	std::string name() const {
		return m_name;
	}

	template <typename T>
	void readFor(const std::chrono::duration<T>& timeout) {
		std::thread th;
		std::unique_lock<std::mutex> lk(m_mu);

		if (!m_recite.test_and_set())
			th = std::thread(&Meter::read, this);

		auto status = m_cv.wait_for(lk, timeout);
		if (th.joinable()) {
			if (status == std::cv_status::timeout)
				th.detach();
			else
				th.join();
		}
	}

	void read() {
		lg::debug("Get value from {}... Write commands:", m_name);
		for (const auto& readCmd : m_cmd.read) {
			m_conn->write(readCmd);
			lg::debug(readCmd);
		}

		auto value = m_conn->read();

		std::lock_guard<std::mutex> lg(m_mu);
		if (!value.empty())
			m_value = value;
		m_cv.notify_all();
		m_recite.clear();
	}

	std::string get() {
		std::lock_guard<std::mutex> lg(m_mu);
		return m_value;
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