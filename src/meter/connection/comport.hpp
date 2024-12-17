#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>

#include <CSerialPort/SerialPort.h>
#include <CSerialPort/SerialPortInfo.h>

#include "connection.hpp"

using namespace itas109;
namespace chrono = std::chrono;

namespace COM {
	class Listener : public CSerialPortListener {
		std::weak_ptr<CSerialPort> m_sp;
		std::string m_buffer;
		std::string m_answer;
		std::condition_variable m_cv;
		chrono::duration<double> m_timeout;
	public:
		std::mutex m_mu;
		Listener(std::shared_ptr<CSerialPort> sp)
			: m_sp(sp)
			, m_buffer{}
			, m_answer{}
			, m_mu{}
			, m_cv{}
			, m_timeout{2.0}
		{}
		void onReadEvent(const char* portName, unsigned int readBufferLen) override;
		void saveBuffer(const std::string& buf);
		std::string lastAnswer();
	};
};

class Comport : public Connection {
	std::shared_ptr<CSerialPort> m_sp;
	COM::Listener m_listener;

public:
	explicit Comport(const std::string& port);
	~Comport();

	void write(const std::string& msg) final;
	std::string read() final {
		return m_listener.lastAnswer();
	}
};