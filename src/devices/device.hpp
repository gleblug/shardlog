#pragma once

#include "config/config_manager.hpp"

class Device {
public:
    Device(const DeviceInfo& info);

private:
    std::string name_;
    std::string port_;
    unsigned int boudRate_;

    double readTimeout_;
    double readWriteDelay_;
    std::string writeSource_;
    
    CommandList initCommands_;
    std::map<std::string, CommandList> readCommands_;
    std::map<std::string, CommandList> writeCommands_;
    CommandList endCommands_;
};
