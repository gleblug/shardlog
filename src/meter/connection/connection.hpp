#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <cstring>

#include <spdlog/spdlog.h>
#include <better-enums/enum.h>
#include <visa.h>
#include <visatype.h>
#include <CSerialPort/SerialPort.h>
#include <CSerialPort/SerialPortInfo.h>

namespace lg = spdlog;
using namespace itas109;

BETTER_ENUM(ConnectionType, uint8_t,
	NIVISA,
	COM
);

class Connection {
protected:
	std::string m_port;
public:
	explicit Connection(const std::string& port) : m_port(port) {}
	virtual ~Connection() = default;
	virtual void write(const std::string& msg) = 0;
	virtual std::string read() = 0;
	virtual std::string query(const std::string& msg) {
		write(msg);
		return read();
	}

	template <typename Derived>
	static std::unique_ptr<Connection> open(const std::string& port) {
		return std::make_unique<Derived>(port);
	}
};

namespace COM {
	class Listener : public CSerialPortListener {
		std::weak_ptr<CSerialPort> m_sp;
		std::string m_buffer;
		std::string m_answer;
		std::condition_variable m_cv;
	public:
		std::mutex m_mu;
		Listener(std::shared_ptr<CSerialPort> sp)
			: m_sp(sp)
			, m_buffer{}
			, m_answer{}
			, m_mu{}
			, m_cv{}
		{}
		void onReadEvent(const char* portName, unsigned int readBufferLen) override {
			char* data = new char[readBufferLen];
			if (readBufferLen > 0 && data) {
				int recLen;
				{
					std::lock_guard<std::mutex> lg(m_mu);
					recLen = m_sp.lock()->readData(data, readBufferLen);
				}
				if (recLen > 0) {
					m_buffer += std::string(data, readBufferLen);
					auto eolIdx = m_buffer.find("\n");
					if (eolIdx != std::string::npos) {
						saveAnswer(eolIdx);
						m_buffer = m_buffer.substr(eolIdx + 1);
					}
				}
			}
			delete[] data;
		}
		void saveAnswer(const size_t eolIdx) {
			std::lock_guard<std::mutex> lg(m_mu);
			m_answer = m_buffer.substr(0, eolIdx);
			m_cv.notify_all();
		}
		std::string lastAnswer() {
			std::string res = "";
			std::unique_lock<std::mutex> lk(m_mu);

			std::swap(res, m_answer);
			if (!res.empty())
				return res;

			if (m_cv.wait_for(lk, std::chrono::seconds(1)) == std::cv_status::timeout) {
				lg::warn("COM port timeout");
				return "";
			}

			std::swap(res, m_answer);
			return res;
		}
	};
};

class Comport : public Connection {
	std::shared_ptr<CSerialPort> m_sp;
	COM::Listener listener;
	std::chrono::milliseconds timeout;

public:
	explicit Comport(const std::string& port)
		: Connection(port)
		, m_sp{ new CSerialPort }
		, listener{ m_sp }
		, timeout{ std::chrono::milliseconds(5000) }
	{
		m_sp->init(m_port.c_str());
		m_sp->setReadIntervalTimeout(1);
		m_sp->setMinByteReadNotify(10);
		if (!m_sp->open())
			lg::error("Failed to open '{}'!");
		m_sp->connectReadEvent(&listener);
	}
	~Comport() {
		m_sp->disconnectReadEvent();
		m_sp->flushBuffers();
		m_sp->close();
	}
	void write(const std::string& msg) final {
		auto msgCopy = msg + "\n\r";
		std::lock_guard<std::mutex> lg(listener.m_mu);
		m_sp->writeData(msgCopy.c_str(), msgCopy.size());
		std::this_thread::sleep_for(std::chrono::duration<double>(10e-3));
	}
	std::string read() final {
		return listener.lastAnswer();
	}
};

namespace Visa {
	class ResourceManager {
		ViSession m_rm;
	public:
		ResourceManager() {
			if (viOpenDefaultRM(&m_rm) < VI_SUCCESS) {
				lg::error("Can't open VISA resource manager!");
				return;
			}
			lg::debug("A new session of VISA resource manager was opened.");
		}

		~ResourceManager() {
			if (viClose(m_rm) < VI_SUCCESS)
				lg::error("Error while closing VISA resource manager!");
		}

		std::vector<std::string> resourcesList() {
			ViChar desc[256];
			ViUInt32 itemCnt;
			ViFindList list;
			if (viFindRsrc(m_rm, (ViString)"USB?*INSTR", &list, &itemCnt, desc) < VI_SUCCESS) {
				lg::error("Can't find resources on resource manager!");
				return {};
			}

			std::vector<std::string> resList;
			for (std::size_t i = 0; i < itemCnt; ++i) {
				resList.emplace_back(desc);
				switch (viFindNext(list, desc)) {
				case VI_SUCCESS:
					continue;
				case VI_ERROR_RSRC_NFOUND:
					break;
				default:
					lg::error("Error while finding resources!");
					break;
				}
			}
			return resList;
		}
		ViSession openResource(const std::string& port) {
			ViSession vi;
			if (viOpen(m_rm, (ViRsrc)port.c_str(), VI_NULL, VI_NULL, &vi) < VI_SUCCESS) {
				lg::error("Can't open resource {}!", port);
				return 0;
			}
			return vi;
		}
	};
};

class Nivisa : public Connection {
	Visa::ResourceManager m_rm;
	ViSession m_vi;
public:
	explicit Nivisa(const std::string& port)
		: Connection(port)
		, m_rm{}
		, m_vi{ m_rm.openResource(m_port) }
	{}

	~Nivisa() override {
		if (viClose(m_vi) < VI_SUCCESS)
			lg::error("Error while closing VISA connection!");
	}

	void write(const std::string& msg) final {
		ViUInt32 retCnt;
		if (viWrite(m_vi, (ViBuf)msg.c_str(), (ViUInt32)msg.size(), &retCnt) < VI_SUCCESS)
			lg::error("Can't write to {}!", m_port);
	}

	std::string read() final {
		ViUInt32 retCnt;
		ViChar buf[256];
		if (viRead(m_vi, (ViBuf)buf, sizeof(buf), &retCnt) != VI_SUCCESS) {
			lg::error("Can't read from {}!", m_port);
			return "";
		}
		buf[retCnt] = '\0';
		return buf;
	}
};