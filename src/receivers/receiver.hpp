#pragma once

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual void receive(const DeviceDataEvent& event) = 0;
    virtual void configure(const std::unordered_map<std::string, std::string> &params) {};
};
