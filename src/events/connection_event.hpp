#pragma once

#include <string>

struct ConnectionEvent {
    enum class Type {
        READY,
        TIMEOUT,
        CONNECTED,
        DISCONNECTED,
    } type;
    
    std::string port;
};
