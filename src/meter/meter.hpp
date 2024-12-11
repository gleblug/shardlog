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
		using Name = std::string;
		using List = std::vector<std::string>;
		using NamedList = std::pair<Name, List>;

		Name name;
		List conf;
		std::vector<NamedList> read;
		List set;
		List end;
	};

	struct SetValue {
		chrono::duration<double> timePoint;
		std::vector<std::string> args;
	};

	using Name = std::string;
	using Value = std::string;
	using Values = std::unordered_map<Commands::Name, Value>;
	using List = std::vector<std::unique_ptr<Meter>>;

	struct Config {
		Name name;
		Commands commands;
		ConnectionType connectionType;
		std::string port;
		std::vector<SetValue> setData;
	};

private:
	Name m_name;
	Commands m_cmd;
	std::unique_ptr<Connection> m_conn;
	std::atomic_flag m_recite;
	std::mutex m_mu;
	std::condition_variable m_cv;
	Values m_values;
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
		, m_values{}
		, m_averageResponseTime{ 0 }
		, m_responseCount{ 0 }
		, m_setData{setData}
		, m_currSetIdx{ 0 }
	{
		for (const auto& title : readTitles())
			m_values[title] = "0";

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

	std::vector<std::string> readTitles() const {
		std::vector<std::string> res;
		std::transform(m_cmd.read.cbegin(), m_cmd.read.cend(), std::back_inserter(res), [](const Commands::NamedList& list) { return list.first; });
		return res;
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
			auto argCmd = cmd + currentSetValue().args.at(0);
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
		lg::debug("Get values from {}... Write commands:", m_name);
		auto startTime = chrono::steady_clock::now();
		std::unordered_map<std::string, std::string> values{};

		for (const auto& readCommands : m_cmd.read) {
			for (const auto& cmd : readCommands.second) {
				m_conn->write(cmd);
				lg::debug("Write to {}: {}", m_name, cmd);
			}
			values[readCommands.first] = m_conn->read();
		}

		std::lock_guard<std::mutex> lg(m_mu);
		for (const auto& [key, val] : values)
			if (!val.empty())
				m_values.at(key) = val;
		if (m_responseCount < 100) {
			m_averageResponseTime += (chrono::steady_clock::now() - startTime);
			++m_responseCount;
		}
		m_cv.notify_all();
		m_recite.clear();
	}

	std::unordered_map<std::string, std::string> get() {
		std::lock_guard<std::mutex> lg(m_mu);
		return m_values;
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