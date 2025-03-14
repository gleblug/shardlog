#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"
#include "events/command_event.hpp"

#include "components/devices_status.hpp"

using namespace ftxui;

struct ToggledComponent {
    Component component;
    bool enabled;
};

class ConsoleFrontend {
public:
    ConsoleFrontend(std::shared_ptr<EventBus<CommandEvent>> commandBus);
    void run();
    void handleConnection(const ConnectionEvent& event);
    
private:
    std::shared_ptr<EventBus<CommandEvent>> commandBus_;

    ScreenInteractive screen_;

    DevicesStatus devicesStatus_;
    // void devicesStatus();

    // Component startExperiment();
    // MenuData startExperiment_;
};
