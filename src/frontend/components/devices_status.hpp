#pragma once

#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"

#include <ftxui/component/component.hpp>
#include <memory>
#include <unordered_map>

using namespace ftxui;

class DevicesStatus {
friend class ConsoleFrontend;
public:
    DevicesStatus() = default;

    void addPort(const std::string& port, ConnectionEvent::Type type);
    void removePort(const std::string& port);

    Component componentReady();
    Component componentConnected();

private:
    std::unordered_map<std::string, ConnectionEvent::Type> usedPorts_;
};
