#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"
#include "events/command_event.hpp"
#include "events/data_event.hpp"

using namespace ftxui;

class ConsoleFrontend {
public:
    ConsoleFrontend(CommandBus commandBus);
    void run();
    void handleConnection(const ConnectionEvent& event);
    void handleData(const DataEvent& event);
    
private:
    std::mutex mu_;

    CommandBus commandBus_;
    ScreenInteractive screen_;

    std::unordered_map<std::string, ConnectionEvent::Type> portsStatus_;

    Component Measurements();
    Component Connections();
};
