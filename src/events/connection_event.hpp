#pragma once

#include <string>
#include <memory>

#include "event_bus/event_bus.hpp"

struct ConnectionEvent {
    enum class Type {
        READY,
        TIMEOUT,
        CONNECTED,
        DISCONNECTED,
    } type;
    
    std::string port;
};

using ConnectionBus = std::shared_ptr<EventBus<ConnectionEvent>>;
