#include "meter.hpp"

Meter::Meter(const std::string& name, const std::string& port, const std::vector<SetValue>& setData, const Commands& cmd)
	: m_name{ name }
	, m_conn{ Connection::openAuto(port) }
	, m_setData{ setData }
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
	lg::debug("{} trying to set data...", m_name);
	for (const auto& cmd : m_cmd.set) {
		auto argCmd = cmd + currentSetValue().args.at(0);
		m_conn->write(argCmd);
		lg::debug("Set command: {}", argCmd);
	}
	++m_currSetIdx;
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