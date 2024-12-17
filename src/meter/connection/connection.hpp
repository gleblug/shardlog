#pragma once

#include <string>
#include <memory>

class Connection {
protected:
	std::string m_port;
public:
	enum class Type {
		UNKNOWN,
		NIVISA,
		COM
	};

	explicit Connection(const std::string& port) : m_port(port) {}
	virtual ~Connection() = default;

	virtual void write(const std::string& msg) = 0;
	virtual std::string read() = 0;
	virtual std::string query(const std::string& msg) {
		write(msg);
		return read();
	}

	template <typename Derived>
	static std::unique_ptr<Connection> open(const std::string& port);
	static std::unique_ptr<Connection> openAuto(const std::string& port);
	static Type type(const std::string& port);
};

