#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "event_bus/event_bus.hpp"

enum class ConnectionType {
    DISCONNECTED,
    AVAILABLE,
    CONNECTED,
    TIMEOUT,
};

inline std::string connectionStatus(const ConnectionType type) {
    std::unordered_map<ConnectionType, std::string> statuses = {
        {ConnectionType::DISCONNECTED, "Disconnected"},
        {ConnectionType::AVAILABLE, "Available"},
        {ConnectionType::CONNECTED, "Connected"},
        {ConnectionType::TIMEOUT, "Timeout"},
    };
    if (statuses.contains(type)) return statuses.at(type);
    return "Unknown";
}

struct ConnectionEvent {
    std::string port;
    ConnectionType type;
};

using ConnectionBus = std::shared_ptr<EventBus<ConnectionEvent>>;
