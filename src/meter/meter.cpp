#include "meter.hpp"

#include <exception>

#include <xtd/ustring.h>

using xtd::ustring;

Meter::Meter(const std::string& name, const std::string& port, const SetArguments& setArgs, const Commands& cmd)
	: m_name{ name }
	, m_conn{ Connection::openAuto(port) }
	, m_setArgs{ setArgs }
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
	lg::debug("{} meter configs:\n"
		"  - port: {}\n"
		"  - setArgs size: {}\n",
		m_name, port, setArgs.size()
	);

	lg::debug("Connect to {}...", m_name);
	if (!ready()) {
		throw std::runtime_error(std::format("Can't connect to {}!", m_name));
	}
	
	for (const auto& cmd : m_cmd.conf) {
		lg::debug("{} config command: {}", m_name, cmd);
		m_conn->write(cmd);
	}
}

Meter::~Meter() {
	m_thread.join();
	for (const auto& cmd : m_cmd.end) {
		lg::debug("{} end command: {}", m_name, cmd);
		m_conn->write(cmd);
	}
}

std::vector<std::string> Meter::readTitles() const {
	std::vector<std::string> res;
	std::transform(m_cmd.read.cbegin(), m_cmd.read.cend(), std::back_inserter(res), [](const Commands::NamedList& list) { return list.first; });
	return res;
}

void Meter::setCurrentData() {
	std::lock_guard lg{ m_mu };
	for (const auto& cmd : m_cmd.set) {
		auto setCmd = cmd;
		bool parseSuccess = true;
		auto openBrace = setCmd.find_first_of('{');
		while (openBrace != std::string::npos) {
			auto closeBrace = setCmd.find_first_of('}');
			if (closeBrace == std::string::npos) {
				lg::warn("{}: Invalid set cmd! {}", m_name, cmd);
				parseSuccess = false;
				break;
			}
			auto argIdx = ustring::parse<size_t>(setCmd.substr(openBrace + 1, closeBrace - openBrace - 1));
			if (argIdx >= m_setArgs.argsCount()) {
				lg::warn("{}: Too large idx in set cmd! {}", m_name, cmd);
				parseSuccess = false;
				break;
			}
			setCmd = setCmd.substr(0, openBrace) + m_setArgs.currentArgs().at(argIdx) + setCmd.substr(closeBrace + 1);
			openBrace = setCmd.find_first_of('{');
		}
		if (parseSuccess) {
			lg::debug("{} set command: {}", m_name, setCmd);
			m_conn->write(setCmd);
		}
	}
	m_setArgs.next();
}

void Meter::readUntil(const chrono::steady_clock::time_point& time_point) {
	if (!m_future.valid() || m_future.wait_for(0ms) == std::future_status::ready) {
		if (m_thread.joinable())
			m_thread.join();
		std::packaged_task<void()> task(std::bind(&Meter::read, this));
		m_future = task.get_future();
		m_thread = std::thread(std::move(task));
	}

	m_future.wait_until(time_point);
}

void Meter::read() {
	lg::debug("Get values from {}... Write commands:", m_name);
	auto startTime = chrono::steady_clock::now();
	std::unordered_map<std::string, std::string> values{};

	std::lock_guard lg{ m_mu };
	for (const auto& readCommands : m_cmd.read) {
		for (const auto& cmd : readCommands.second) {
			m_conn->write(cmd);
			lg::debug("Write to {}: {}", m_name, cmd);
		}
		values[readCommands.first] = m_conn->read();
	}

	for (const auto& [key, val] : values)
		if (!val.empty())
			m_values.at(key) = val;
	if (m_responseCount < 100) {
		m_averageResponseTime += (chrono::steady_clock::now() - startTime);
		++m_responseCount;
	}
}