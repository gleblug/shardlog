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
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <vector>
#include <string>
#include <format>

using namespace ftxui;
namespace lg = spdlog;

ConsoleFrontend::ConsoleFrontend(CommandBus commandBus)
    : commandBus_{commandBus}
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
        Container::Tab({
            Connections(),
            Terminal()
            }, &connectionSelected_)
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
    devicesResult_ = event.results;
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
    return Renderer(mainComponent, [this, &config, mainComponent] {
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

        auto desk = Container::Vertical({
            cc::DevicesResultComponent(devicesResult_) | flex,
            Container::Horizontal({
                Button((measuring_ ? "Stop" : "Start"), [this]{
                    commandBus_->publish(CommandEvent{
                        measuring_ ? CommandType::STOP_MEASUREMENT : CommandType::START_MEASUREMENT
                    });
                    measuring_ = !measuring_;
                }) | size(WIDTH, EQUAL, 10),
                Renderer([this] {
                    return text("PLACEHOLDER FOR GAUGE");
                }) | flex
            })
        });

        mainComponent->Add(Container::Vertical(experiments));
        mainComponent->Add(desk);
        return mainComponent->Render();
    });
}

Component ConsoleFrontend::Connections() {
    auto terminalButton = Button("Terminal", [this]{ connectionSelected_ = 1; });
    return Renderer(terminalButton, [terminalButton, this]{
        Elements ports;
        for (const auto& [port, status] : portsStatus_) {
            if (status == ConnectionEvent::Type::DISCONNECTED) {
                continue;
            }
            ports.push_back(hbox({
                text(port) | bold | flex,
                text("Not ready"),
            }) | border);
        }
        return vbox({
            vbox(ports) | flex,
            terminalButton->Render()
        });
    });
}

Component ConsoleFrontend::Terminal() {
    // auto connection = std::make_shared<Serial>(port, boudrate);
    auto outputArray = std::make_shared<std::vector<std::string>>();
    auto inputString = std::make_shared<std::string>();
    
    // control
    auto control = Container::Horizontal({
        Button(" < ", [this]{ connectionSelected_ = 0; })
    });

    // output
    auto consoleOutput = cc::ScrollableTextArea(outputArray);

    // input
    auto inputStyle = InputOption::Default();
    inputStyle.on_enter = [outputArray, inputString]{
        auto newLine = std::format("> {}", *inputString);
        outputArray->push_back(newLine);
        inputString->clear();
    };
    auto consoleInput = Input(inputString.get(), inputStyle) | border;

    // result
    return Container::Vertical({
        control,
        consoleOutput,
        consoleInput
    });
}
