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

    std::vector<std::string> tabValues = {"Measurement", "Connection", "Quit"};
    int tabSelected = 0;
    auto tabMenu = cc::NamedMenu("Menu", tabValues, tabSelected, {{tabValues.size() - 1, [&]{ quitModalShown = true; }}});
    auto tabContainer = Container::Tab({
        Measurements(),
        Connections(),
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

void ConsoleFrontend::activateMeasurement(const std::string& experimentName, const std::string& measurementName) {
    commandBus_->publish(CommandEvent{
        CommandType::ACTIVATE_MEASUREMENT,
        {{"experiment_name", experimentName}, {"measurement_name", measurementName}}
    });
    measurementsSelected_ = 1;
}

Component ConsoleFrontend::Measurements() {
    auto& config = ConfigManager::getInstance();
    auto mainComponent = Container::Tab({}, &measurementsSelected_);
    return Renderer(mainComponent, [this, &config, mainComponent]{
        Components experiments;
        for (const auto &experimentName : config.getExperimentNames()) {
            Components measurements;
            for (const auto &measurementName : config.getMeasurementNames(experimentName)) {
                measurements.push_back(Button(measurementName, [this, experimentName, measurementName]{
                    activateMeasurement(experimentName, measurementName);
                }));
            }
            experiments.push_back(Collapsible(experimentName, cc::CollapsibleInner(measurements)));
        }

        mainComponent->Add(Container::Vertical(experiments));
        mainComponent->Add(Container::Vertical({}));
        return mainComponent->Render();
    });
}

Component ConsoleFrontend::Connections() {
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
