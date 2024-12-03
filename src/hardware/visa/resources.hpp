#pragma once
#include <spdlog/spdlog.h>
#include <visa.h>
#include <visatype.h>

namespace lg = spdlog;

class Resource {

	ViSession vi;
	std::string addr;

public:
	const std::string idn;

	Resource(const ViSession _vi, const std::string& _addr) : vi(_vi), addr(_addr), idn(query("*IDN?")) {}
	~Resource() { viClose(this->vi); }
	void write(const std::string& buf) {
		ViUInt32 retCnt;
		if (viWrite(vi, (ViBuf)buf.c_str(), (ViUInt32)buf.size(), &retCnt) != VI_SUCCESS)
			lg::error("Can't write to {}", this->addr);
	}
	std::string read() {
		ViUInt32 retCnt;
		ViChar buf[256];
		if (viRead(vi, (ViBuf)buf, sizeof(buf), &retCnt) != VI_SUCCESS) {
			lg::error("Can't read from {}", this->addr);
			return "";
		}
		buf[retCnt] = '\0';
		return buf;
	}
	std::string query(const std::string& buf) {
		write(buf);
		return read();
	}
};

class ResourceManager {
	inline static ViSession rm;
	inline static bool opened;
public:
	ResourceManager() {
		if (opened)
			return;

		if (viOpenDefaultRM(&rm) != VI_SUCCESS) {
			lg::error("Can't open VISA resource manager!");
			opened = false;
			return;
		}

		lg::info("the visa resource manager is now open");
		opened = true;
	}

	~ResourceManager() {}

	static void close() {
		if (!opened)
			return;
		if (viClose(rm) != VI_SUCCESS)
			lg::error("Error while closing resource manager!");
		opened = false;
	}

	std::vector<std::string> resourcesList() {
		if (!this->opened) {
			lg::error("Resource manager is closed!");
			return {};
		}

		ViChar desc[256];
		ViUInt32 itemCnt;
		ViFindList list;
		if (viFindRsrc(this->rm, (ViString)"USB?*INSTR", &list, &itemCnt, desc) != VI_SUCCESS) {
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

	std::unique_ptr<Resource> openResource(const std::string& desc) {
		ViSession vi;
		if (viOpen(rm, (ViRsrc)desc.c_str(), VI_NULL, VI_NULL, &vi) != VI_SUCCESS) {
			lg::error("Can't open resource {}", desc);
			return nullptr;
		}
		return std::make_unique<Resource>(vi, desc);
	}
};