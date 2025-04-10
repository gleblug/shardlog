#pragma once

#include "events/data_event.hpp"

#include <string>

enum class ReceiverType {
    DIRECTORY,
    TELEGRAM,
    UNKNOWN
};

struct ReceiverInfo {
    ReceiverType type;
    std::string name;
};

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual void receive(const DataEvent& event) = 0;
    virtual void renew(const std::string& name) = 0;
};
