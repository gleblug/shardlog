#include "nivisa.hpp"

#include <spdlog/spdlog.h>

#include "connection.hpp"

namespace lg = spdlog;

using namespace Visa;

ResourceManager::ResourceManager() {
	if (viOpenDefaultRM(&m_rm) < VI_SUCCESS) {
		lg::error("Can't open VISA resource manager!");
		return;
	}
	lg::debug("A new session of VISA resource manager was opened.");
}

ResourceManager::~ResourceManager() {
	if (viClose(m_rm) < VI_SUCCESS)
		lg::error("Error while closing VISA resource manager!");
}

std::vector<std::string> ResourceManager::resourcesList() {
	ViChar desc[256];
	ViUInt32 itemCnt;
	ViFindList list;
	if (viFindRsrc(m_rm, (ViString)"USB?*INSTR", &list, &itemCnt, desc) < VI_SUCCESS) {
		lg::debug("Can't find resources on resource manager!");
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

ViSession ResourceManager::openResource(const std::string& port) {
	ViSession vi;
	if (viOpen(m_rm, (ViRsrc)port.c_str(), VI_NULL, VI_NULL, &vi) < VI_SUCCESS)
		throw std::runtime_error(std::format("Unable to open {}", port));

	return vi;
}

Nivisa::~Nivisa() {
	if (viClose(m_vi) < VI_SUCCESS)
		lg::error("Error while closing VISA connection!");
}

void Nivisa::write(const std::string& msg) {
	ViUInt32 retCnt;
	if (viWrite(m_vi, (ViBuf)msg.c_str(), (ViUInt32)msg.size(), &retCnt) < VI_SUCCESS)
		lg::error("Can't write to {}!", m_port);
}

std::string Nivisa::read() {
	ViUInt32 retCnt;
	ViChar buf[256];
	if (viRead(m_vi, (ViBuf)buf, sizeof(buf), &retCnt) != VI_SUCCESS) {
		lg::error("Can't read from {}!", m_port);
		return "";
	}
	buf[retCnt] = '\0';
	std::string res(buf);
	return res.substr(0, res.find_last_of('\n'));
}