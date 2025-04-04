#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "event_bus/event_bus.hpp"

struct CommandEvent {
    enum class Type {
        ACTIVATE_MEASUREMENT,
        START_MEASUREMENT,
        STOP_MEASUREMENT,
    } type;

    std::unordered_map<std::string, std::string> parameters;
};

using CommandBus = std::shared_ptr<EventBus<CommandEvent>>;
using CommandType = CommandEvent::Type;
