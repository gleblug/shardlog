#include "connection.hpp"
#include "nivisa.hpp"
#include "comport.hpp"

#include <atomic>
#include <thread>

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;
using xtd::ustring;

template <typename Derived>
std::unique_ptr<Connection> Connection::open(const std::string& port) {
	return std::make_unique<Derived>(port);
}

std::unique_ptr<Connection> Connection::openAuto(const std::string& port) {
	switch (type(port)) {
	case Type::NIVISA:
		return open<Nivisa>(port);
	case Type::COM:
		return open<Comport>(port);
	default:
		throw std::runtime_error("Unknown connection type!");
	}
}

Connection::Type Connection::type(const std::string& port) {
	if (ustring(port).to_lower().contains("com"))
		return Type::COM;
	return Type::NIVISA;
}