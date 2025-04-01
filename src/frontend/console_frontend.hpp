#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"
#include "events/command_event.hpp"

using namespace ftxui;

struct ToggledComponent {
    Component component;
    bool enabled;
};

class ConsoleFrontend {
public:
    ConsoleFrontend(CommandBus commandBus);
    void run();
    // void handleConnection(const ConnectionEvent& event);
    
private:
    CommandBus commandBus_;

    ScreenInteractive screen_;
};
