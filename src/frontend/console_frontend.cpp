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
    : mu_{}
    , commandBus_{commandBus}
    , screen_{ScreenInteractive::Fullscreen()}
    , portsStatus_{}
{}

void ConsoleFrontend::run() {
    bool quitModalShown = false;
    auto modalQuit = cc::ModalConfirmation(screen_.ExitLoopClosure(), [&]{ quitModalShown = false; });

    std::vector<std::string> tabValues = {"Measurement", "Devices", "Quit"};
    int tabSelected = 0;
    auto tabMenu = cc::NamedMenu("Menu", tabValues, tabSelected, {{tabValues.size() - 1, [&]{ quitModalShown = true; }}});
    auto tabContainer = Container::Tab({
        Measurements(),
        Devices(),
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

void ConsoleFrontend::handleConnection(const ConnectionEvent& event) {
    switch (event.type) {
    case ConnectionEvent::Type::DISCONNECTED:
        portsStatus_.erase(event.port);
        break;
    default:
        portsStatus_.insert_or_assign(event.port, event.type);
        break;
    }
    screen_.RequestAnimationFrame();
}

void ConsoleFrontend::handleData(const DataEvent& event) {
    screen_.RequestAnimationFrame();
}

Component ConsoleFrontend::Measurements() {
    auto& config = ConfigManager::getInstance();
    auto names = config.getExperimentNames();
    ButtonOption buttonOption;
    buttonOption.transform = [](EntryState state) {
        state.label = (state.focused ? "> " : "  ") + state.label;
        Element e = text(state.label) | border | size(HEIGHT, EQUAL, 3);
        return e;
    };

    Components buttons;
    for (const auto name : names) {
        buttons.push_back(Button(name, []{}, buttonOption));
    }

    return Container::Vertical(buttons);
}

Component ConsoleFrontend::Devices() {
    return Renderer([this]{
        Elements ports;
        for (const auto& [port, status] : portsStatus_) {
            if (status == ConnectionEvent::Type::DISCONNECTED) {
                continue;
            }
            ports.push_back(vbox({
                hbox(text(" ●  ") | color(Color::GrayDark), text(port)) | bold,
                separator(),
                text("Not ready"),
            }) | border | size(WIDTH, EQUAL, 20));
        }
        return flexbox(ports);
    });
}
