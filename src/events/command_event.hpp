#pragma once

#include <string>
#include <unordered_map>

struct CommandEvent {
    enum class Type {
        START,
        STOP,
        SET,
    } type;

    std::unordered_map<std::string, std::string> parameters;
};
