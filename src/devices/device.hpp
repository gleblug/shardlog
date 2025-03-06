#pragma once

class IDevice {
public:
    virtual ~IDevice() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void configure(const std::unordered_map<std::string, std::string> &params) {};
};
