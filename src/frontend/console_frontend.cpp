#include "console_frontend.hpp"

// #include "components/modal.hpp"
// #include "components/measurement.hpp"
// #include "components/device.hpp"
// #include "components/menu.hpp"
#include "components.hpp"

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <vector>
#include <string>

using namespace ftxui;
namespace lg = spdlog;

ConsoleFrontend::ConsoleFrontend(CommandBus commandBus)
    : commandBus_{commandBus}
    , screen_{ScreenInteractive::Fullscreen()}
{}

void ConsoleFrontend::run() {
    bool quitModalShown = false;
    auto modalQuit = cc::ModalConfirmation(screen_.ExitLoopClosure(), [&]{ quitModalShown = false; });

    std::vector<std::string> tabValues = {"Measurement", "Devices", "Quit"};
    int tabSelected = 0;
    auto tabMenu = cc::NamedMenu("Menu", tabValues, tabSelected, {{tabValues.size() - 1, [&]{ quitModalShown = true; }}});
    auto tabContainer = Container::Tab({
        // cc::Measurements(),
        // devicesStatus_.componentConnected()
    }, &tabSelected);

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

// void ConsoleFrontend::handleConnection(const ConnectionEvent& event) {
//     switch (event.type) {
//     case ConnectionEvent::Type::DISCONNECTED:
//         // devicesStatus_.removePort(event.port);
//         break;
//     default:
//         // devicesStatus_.addPort(event.port, event.type);
//         break;
//     }
//     screen_.RequestAnimationFrame();
// }
