#include "resources.hpp"

void Resource::write(const std::string& buf) {
	ViUInt32 retCnt;
	if (viWrite(vi, (ViBuf)buf.c_str(), (ViUInt32)buf.size(), &retCnt) != VI_SUCCESS)
		lg::error("Can't write to {}", this->addr);
}

std::string Resource::read() {
	ViUInt32 retCnt;
	ViChar buf[256];
	if (viRead(vi, (ViBuf)buf, sizeof(buf), &retCnt) != VI_SUCCESS) {
		lg::error("Can't read from {}", this->addr);
		return "";
	}
	buf[retCnt] = '\0';
	return buf;
}

std::string Resource::query(const std::string& buf) {
	write(buf);
	return read();
}

ResourceManager::ResourceManager() {
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

void ResourceManager::close() {
	if (!opened)
		return;
	if (viClose(rm) != VI_SUCCESS)
		lg::error("Error while closing resource manager!");
	opened = false;
}

std::vector<std::string> ResourceManager::resourcesList() {
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

std::unique_ptr<Resource> ResourceManager::openResource(const std::string& desc) {
	ViSession vi;
	if (viOpen(rm, (ViRsrc)desc.c_str(), VI_NULL, VI_NULL, &vi) != VI_SUCCESS) {
		lg::error("Can't open resource {}", desc);
		return nullptr;
	}
	return std::make_unique<Resource>(vi, desc);
}