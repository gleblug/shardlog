#pragma once

#include "config/config_manager.hpp"
#include "connection/serial.hpp"

class Device {
public:
    Device(const DeviceInfo& info);

    void open();

private:
    std::string name_;
    std::string port_;
    unsigned int boudRate_;

    std::unique_ptr<Serial> connection_;

    double readTimeout_;
    double readWriteDelay_;
    std::string writeSource_;

    CommandList initCommands_;
    NamedCommandList readCommands_;
    NamedCommandList writeCommands_;
    CommandList endCommands_;
};
