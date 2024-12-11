#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <xtd/ustring.h>
#include <fmt/format.h>

#include "connection/connection.hpp"

namespace lg = spdlog;
namespace fs = std::filesystem;
namespace chrono = std::chrono;

class Meter {
public:
	struct Commands {
		std::string name;
		std::vector<std::string> conf;
		std::vector<std::string> read;
		std::vector<std::string> set;
		std::vector<std::string> end;
	};

	struct SetValue {
		chrono::duration<double> time;
		std::string arg;
	};

	struct Config {
		std::string name;
		Commands commands;
		ConnectionType connectionType;
		std::string port;
		std::vector<SetValue> setData;
	};

	using List = std::vector<std::unique_ptr<Meter>>;

private:
	std::string m_name;
	Commands m_cmd;
	std::unique_ptr<Connection> m_conn;
	std::atomic_flag m_recite;
	std::mutex m_mu;
	std::condition_variable m_cv;
	std::string m_value;
	chrono::duration<double> m_averageResponseTime;
	size_t m_responseCount;

	std::vector<SetValue> m_setData;
	size_t m_currSetIdx;

public:

	Meter(const std::string& name, const Commands& cmd, std::unique_ptr<Connection>&& conn, const std::vector<SetValue>& setData = {})
		: m_name{ name }
		, m_cmd{ cmd }
		, m_conn{ std::move(conn) }
		, m_recite{}
		, m_mu{}
		, m_cv{}
		, m_value{ "0" }
		, m_averageResponseTime{ 0 }
		, m_responseCount{ 0 }
		, m_setData{setData}
		, m_currSetIdx{ 0 }
	{
		lg::debug("{} set data mode: {}", m_name, !m_setData.empty());
		for (const auto& cmd : m_cmd.conf) {
			lg::debug("{} config command: {}", m_name, cmd);
			m_conn->write(cmd);
		}
	}
	
	~Meter() {
		for (const auto& cmd : m_cmd.end) {
			lg::debug("{} end command: {}", m_name, cmd);
			m_conn->write(cmd);
		}
	}

	std::string name() const {
		return m_name;
	}

	bool needToSet() const {
		return (m_currSetIdx < m_setData.size());
	}

	SetValue currentSetValue() const {
		return m_setData.at(m_currSetIdx);
	}

	void setCurrentData() {
		lg::debug("{} trying to set data...", m_name);
		for (const auto& cmd : m_cmd.set) {
			auto argCmd = cmd + currentSetValue().arg;
			m_conn->write(argCmd);
			lg::debug("Set command: {}", argCmd);
		}
		++m_currSetIdx;
	}

	template <typename T>
	void readFor(const chrono::duration<T>& timeout) {
		std::thread th;
		std::unique_lock lk(m_mu);

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
		auto startTime = chrono::steady_clock::now();
	
		for (const auto& readCmd : m_cmd.read) {
			m_conn->write(readCmd);
			lg::debug(readCmd);
		}
		auto value = m_conn->read();

		std::lock_guard<std::mutex> lg(m_mu);
		if (!value.empty())
			m_value = value;
		if (m_responseCount < 100) {
			m_averageResponseTime += (chrono::steady_clock::now() - startTime);
			++m_responseCount;
		}
		m_cv.notify_all();
		m_recite.clear();
	}

	std::string get() {
		std::lock_guard<std::mutex> lg(m_mu);
		return m_value;
	}

	chrono::duration<double> averageResponseTime() {
		return (m_averageResponseTime / m_responseCount);
	}

	static std::unique_ptr<Meter> create(const Config& conf) {
		return std::make_unique<Meter>(
			conf.name,
			conf.commands,
			Connection::fromType(conf.connectionType, conf.port),
			conf.setData
		);
	}
};