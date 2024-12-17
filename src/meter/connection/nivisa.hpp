#pragma once

#include <spdlog/spdlog.h>
#include <visa.h>
#include <visatype.h>

#include "connection.hpp"

namespace lg = spdlog;

namespace Visa {
	class ResourceManager {
		ViSession m_rm;
	public:
		ResourceManager();
		~ResourceManager();

		std::vector<std::string> resourcesList();
		ViSession openResource(const std::string& port);
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
	~Nivisa() override;

	void write(const std::string& msg) final;
	std::string read() final;
};