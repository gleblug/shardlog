#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <thread>
#include <future>
#include <functional>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <xtd/ustring.h>
#include <fmt/format.h>

#include "connection/connection.hpp"

namespace lg = spdlog;
namespace fs = std::filesystem;
namespace chrono = std::chrono;
using namespace std::chrono_literals;

class SetArguments {
	using Argument = std::pair<double, std::vector<std::string>>;

	std::vector<Argument> m_argsList;
	size_t m_currentIdx;
	size_t m_argsCount;
public:
	SetArguments()
		: m_argsList{}
		, m_currentIdx{0}
		, m_argsCount{0} { }
	bool isOver() const {
		return m_currentIdx >= m_argsList.size();
	}
	auto currentTime() const {
		return chrono::duration<double>(m_argsList.at(m_currentIdx).first);
	}
	auto currentArgs() const {
		return m_argsList.at(m_currentIdx).second;
	}
	size_t size() const {
		return m_argsList.size();
	}
	size_t argsCount() const {
		return m_argsCount;
	}
	void append(const Argument& arg) {
		if (m_argsList.size() == 0)
			m_argsCount = arg.second.size();
		else
			m_argsCount = std::min(m_argsCount, arg.second.size());
		m_argsList.push_back(arg);
	}
	void next() {
		++m_currentIdx;
	}
	
};

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

	using Name = std::string;
	using Value = std::string;
	using Values = std::unordered_map<Commands::Name, Value>;
	using Ptr = std::shared_ptr<Meter>;

	struct Config {
		std::string commandsName;
		std::string port;
		std::string setValuesPath;
	};

private:
	Name m_name;
	std::unique_ptr<Connection> m_conn;
	
	SetArguments m_setArgs;
	Commands m_cmd;
	
	Values m_values;
	chrono::duration<double> m_averageResponseTime;
	size_t m_responseCount;

	std::thread m_thread;
	std::future<void> m_future;
	std::mutex m_mu;

public:
	Meter(const std::string& name, const std::string& port, const SetArguments& setArgs, const Commands& cmd);
	~Meter();

	std::string name() const {
		return m_name;
	}
	bool needToSet(const chrono::steady_clock::time_point& startTime) const {
		return (
			!m_setArgs.isOver()
			&& (startTime + m_setArgs.currentTime() <= chrono::steady_clock::now())
		);
	}

	std::vector<std::string> readTitles() const;
	void setCurrentData();
	void readUntil(const chrono::steady_clock::time_point& time_point);
	void read();

	bool ready() {
		for (size_t i = 0; i < 5; ++i) {
			if (m_conn->query("*IDN?") != "")
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return false;
	}
	std::unordered_map<std::string, std::string> get() {
		std::lock_guard<std::mutex> lg(m_mu);
		return m_values;
	}
	chrono::duration<double> averageResponseTime() {
		return (m_averageResponseTime / m_responseCount);
	}
};