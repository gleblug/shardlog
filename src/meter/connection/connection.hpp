#pragma once

#include <string>
#include <memory>

#include <spdlog/spdlog.h>
#include <better-enums/enum.h>
#include <visa.h>
#include <visatype.h>

namespace lg = spdlog;

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
	std::string query(const std::string& msg) {
		write(msg);
		return read();
	}

	template <typename Derived>
	static std::unique_ptr<Connection> open(const std::string& port) {
		return std::make_unique<Derived>(port);
	}
};

namespace COM {

};

class Comport : public Connection {
public:
	explicit Comport(const std::string& port) : Connection(port) {

	}
	virtual void write(const std::string& msg) {}
	virtual std::string read() { return ""; }
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
			lg::info("A new session of VISA resource manager was opened.");
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