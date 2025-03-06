#pragma once

struct CommandEvent {
    enum class Type {
        START_MEASUREMENTS,
        STOP_MEASUREMENTS,
        SET_DEVICE_PARAMETER,
        SET_RECEIVER_PARAMETER,
        UNKNOWN_COMMAND
    } type;

    std::unordered_map<std::string, std::string> parameters;
};
