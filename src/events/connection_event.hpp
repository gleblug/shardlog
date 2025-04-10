#pragma once

#include <string>
#include <memory>

#include "event_bus/event_bus.hpp"

enum class ConnectionType {
    DISCONNECTED,
    AVAILABLE,
    CONNECTED,
    TIMEOUT,
};

struct ConnectionEvent {
    std::string port;
    ConnectionType type;
};

using ConnectionBus = std::shared_ptr<EventBus<ConnectionEvent>>;
