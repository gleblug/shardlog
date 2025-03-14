#pragma once

#include "events/data_event.hpp"

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual void receive(const DataEvent& event) = 0;
    virtual void configure(const std::unordered_map<std::string, std::string> &params) {};
};
