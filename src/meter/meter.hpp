#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <thread>
#include <future>
#include <functional>

#include <spdlog/spdlog.h>
#include <xtd/ustring.h>
#include <fmt/format.h>

#include "connection/connection.hpp"

namespace lg = spdlog;
namespace fs = std::filesystem;
namespace chrono = std::chrono;
using namespace std::chrono_literals;

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
	using Ptr = std::unique_ptr<Meter>;

	struct Config {
		std::string commandsName;
		std::string port;
		std::string setValuesPath;
	};

private:
	Name m_name;
	std::unique_ptr<Connection> m_conn;
	
	std::vector<SetValue> m_setData;
	size_t m_currSetIdx;
	Commands m_cmd;
	
	Values m_values;
	chrono::duration<double> m_averageResponseTime;
	size_t m_responseCount;

	std::thread m_thread;
	std::future<void> m_future;
	std::mutex m_mu;

public:
	Meter(const std::string& name, const std::string& port, const std::vector<SetValue>& setData, const Commands& cmd)
		: m_name{ name }
		, m_conn{ Connection::openAuto(port) }
		, m_setData{setData}
		, m_currSetIdx{ 0 }
		, m_cmd{ cmd }
		, m_values{}
		, m_averageResponseTime{ 0 }
		, m_responseCount{ 0 }
		, m_thread{}
		, m_future{}
		, m_mu{}
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
		m_thread.join();
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

	void readFor(const chrono::duration<double>& timeout) {
		if (!m_future.valid() || m_future.wait_for(0ms) == std::future_status::ready) {
			if (m_thread.joinable())
				m_thread.join();
			std::packaged_task<void()> task(std::bind(&Meter::read, this));
			m_future = task.get_future();
			m_thread = std::thread(std::move(task));
		}

		m_future.wait_for(timeout);
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
	}

	std::unordered_map<std::string, std::string> get() {
		std::lock_guard<std::mutex> lg(m_mu);
		return m_values;
	}

	chrono::duration<double> averageResponseTime() {
		return (m_averageResponseTime / m_responseCount);
	}
};