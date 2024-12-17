#include "comport.hpp"

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;
using xtd::ustring;

using namespace COM;

void Listener::onReadEvent(const char* portName, unsigned int readBufferLen) {
	char* data = new char[readBufferLen];
	if (readBufferLen > 0 && data) {
		int recLen;
		{
			std::lock_guard lg(m_mu);
			recLen = m_sp.lock()->readData(data, readBufferLen);
		}
		if (recLen > 0) {
			m_buffer += std::string(data, readBufferLen);
			auto eolIdx = m_buffer.find_last_of("\n");
			if (eolIdx != std::string::npos) {
				saveBuffer(m_buffer.substr(0, eolIdx));
				m_buffer = m_buffer.substr(eolIdx + 1);
			}
		}
	}
	delete[] data;
}

void Listener::saveBuffer(const std::string& buf) {
	auto trimBuf = xtd::ustring(buf).trim();
	if (!trimBuf.empty()) {
		auto lastEolIdx = trimBuf.find_last_of("\n");
		std::lock_guard lg(m_mu);
		if (lastEolIdx != std::string::npos)
			m_answer = trimBuf.substr(lastEolIdx + 1);
		else
			m_answer = trimBuf;
		m_cv.notify_all();
	}
}

std::string Listener::lastAnswer() {
	std::string res = "";
	std::unique_lock lk(m_mu);

	std::swap(res, m_answer);
	if (!res.empty())
		return res;
	// TODO : fix unlock unown mutex
	if (m_cv.wait_for(lk, std::chrono::seconds(1)) == std::cv_status::timeout) {
		lg::warn("COM port timeout");
		return "";
	}

	std::swap(res, m_answer);
	return res;
}

Comport::Comport(const std::string& port)
	: Connection(port)
	, m_sp{ new CSerialPort }
	, listener{ m_sp }
	, timeout{ chrono::duration<double>(2.0) }
{
	m_sp->init(m_port.c_str());
	m_sp->setReadIntervalTimeout(1);
	m_sp->setMinByteReadNotify(10);
	if (!m_sp->open())
		throw std::runtime_error(std::format("Unable to open {}", m_port));
	m_sp->connectReadEvent(&listener);
}
	
Comport::~Comport() {
	m_sp->disconnectReadEvent();
	m_sp->flushBuffers();
	m_sp->close();
}

void Comport::write(const std::string& msg) {
	auto msgCopy = msg + "\n";
	std::lock_guard lg(listener.m_mu);
	m_sp->writeData(msgCopy.c_str(), msgCopy.size());
	std::this_thread::sleep_for(std::chrono::duration<double>(5e-3));
}