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
	void write(const std::string& buf);
	std::string read();
	std::string query(const std::string& buf);
};

class ResourceManager {

	inline static ViSession rm;
	inline static bool opened;

public:
	ResourceManager();
	static void close();
	std::vector<std::string> resourcesList();
	std::unique_ptr<Resource> openResource(const std::string& desc);
};