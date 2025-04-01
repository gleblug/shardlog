#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "event_bus/event_bus.hpp"

struct CommandEvent {
    enum class Type {
        START,
        STOP,
        SET,
    } type;

    std::unordered_map<std::string, std::string> parameters;
};

using CommandBus = std::shared_ptr<EventBus<CommandEvent>>;
