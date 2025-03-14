#include "console_frontend.hpp"

#include "components/modal.hpp"
#include "components/measurement.hpp"
#include "components/device.hpp"

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include "ftxui/component/loop.hpp"
#include <spdlog/spdlog.h>

#include <iostream>
#include <vector>
#include <string>

using namespace ftxui;
namespace lg = spdlog;

ConsoleFrontend::ConsoleFrontend(std::shared_ptr<EventBus<CommandEvent>> commandBus)
    : commandBus_{commandBus}
    , screen_{ScreenInteractive::Fullscreen()}
    , devicesStatus_{}
{}

void ConsoleFrontend::run() {
    bool quitModalShown = false;
    auto modalQuit = ModalConfirmation(screen_.ExitLoopClosure(), [&]{ quitModalShown = false; });

    std::vector<std::string> tabValues = {"Measurement", "Devices status", "Edit configs", "Extensions", "Quit"};
    int tabSelected = 0;
    MenuOption tabOption;
    tabOption.on_change = [&]{
        if (tabSelected == tabValues.size() - 1) {
            quitModalShown = true;
        }
    };
    tabOption.entries_option.transform = [](EntryState state) -> Element {
        state.label = (state.active ? "> " : "  ") + state.label;
        Element e = text(state.label) | border | size(HEIGHT, EQUAL, 3);
        if (state.active)
            e = e | bold;
        return e;
    };
    auto tabMenu = Menu(&tabValues, &tabSelected, tabOption);
    auto tabContainer = Container::Tab(
        {
            MeasurementList(),
        },
        &tabSelected
    );

    int menu_size = 24;
    auto resizable = ResizableSplitLeft(
        tabMenu,
        tabContainer,
        &menu_size
    );

    auto renderer = Renderer(resizable, [&]{ return resizable->Render() | border; });
    renderer |= Modal(modalQuit, &quitModalShown);
 
    screen_.Loop(renderer);
}

void ConsoleFrontend::handleConnection(const ConnectionEvent& event) {
    switch (event.type) {
    case ConnectionEvent::Type::DISCONNECTED:
        devicesStatus_.removePort(event.port);
        break;
    default:
        devicesStatus_.addPort(event.port, event.type);
        break;
    }
    screen_.RequestAnimationFrame();
}
