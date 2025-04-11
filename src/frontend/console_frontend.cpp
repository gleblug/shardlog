#include "console_frontend.hpp"

#include "components.hpp"
#include "connection/serial.hpp"

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/replace.hpp>

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
        Container::Tab({
            Measurements(),
            Desk()
        }, &measurementsSelected_),
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
    portsStatus_.insert_or_assign(event.port, event.type);
    connectedPorts_.clear();
    std::transform(portsStatus_.cbegin(), portsStatus_.cend(), std::back_inserter(connectedPorts_),
    [](const std::pair<std::string, ConnectionType>& status){
        return status.first;
    });
    screen_.RequestAnimationFrame();
}

void ConsoleFrontend::handleData(const DataEvent& event) {
    devicesData_ = event.results;
    auto duration = event.end - event.start;
    auto passed = event.timestamp - event.start;
    auto remain = event.end - event.timestamp;
    remainS_ = chrono::ceil<chrono::seconds>(remain);
    percentage_ = static_cast<float>(passed.count()) / static_cast<float>(duration.count());
    if (percentage_ > 1.0) percentage_ = 1.0;
    screen_.RequestAnimationFrame();
}

void ConsoleFrontend::activateMeasurement(const std::string& experimentName, const std::string& measurementName) {
    auto& config = ConfigManager::getInstance();
    auto devicesInfo = config.getDevices(experimentName, measurementName);
    for (const auto& info : devicesInfo) {
        devicesData_.insert_or_assign(info.name, DeviceData{info.port, {}});
    }
    commandBus_->publish(CommandEvent{
        CommandType::ACTIVATE_MEASUREMENT,
        {{"experiment_name", experimentName}, {"measurement_name", measurementName}}
    });
    measurementsSelected_ = 1;
}

Component ConsoleFrontend::Measurements() {
    auto& config = ConfigManager::getInstance();
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

    return Container::Vertical(experiments);
}

Component ConsoleFrontend::Desk() {
    // devices
    auto devices = cc::ScrollableWrapper(cc::DevicesComponent(devicesData_, portsStatus_));

    // controls
    ButtonOption controlBtnOption;
    controlBtnOption.on_click = [this]{
        commandBus_->publish(CommandEvent{
            measuring_ ? CommandType::STOP_MEASUREMENT : CommandType::START_MEASUREMENT, {}
        });
        measuring_ = !measuring_;
    };
    controlBtnOption.transform = [this](EntryState state) {
        state.label = measuring_ ? "Stop" : "Start";
        return text(state.label) | border;
    };
    auto controlBtn = Button(controlBtnOption) | size(WIDTH, EQUAL, 10);

    auto controls = Container::Horizontal({
        controlBtn,
        Renderer([this] {
            return hbox({
                gauge(percentage_) | flex,
                separator(),
                text(std::format("{}s", remainS_.count()))
            });
        }) | border | flex
    });

    return Container::Vertical({
        devices,
        controls
    });
}

Component ConsoleFrontend::Connections() {
    auto terminalButton = Button("Terminal", [this]{ connectionSelected_ = 1; });
    return Renderer(terminalButton, [terminalButton, this]{
        Elements ports;
        for (const auto& [port, status] : portsStatus_) {
            if (status == ConnectionType::DISCONNECTED) {
                continue;
            }
            ports.push_back(hbox({
                text(port) | bold | flex,
                text("Available"),
            }) | border);
        }
        return vbox({
            vbox(ports) | flex,
            terminalButton->Render()
        });
    });
}

Component ConsoleFrontend::Terminal() {
    auto connection = std::make_shared<Serial>();
    connection->setTimeout(boost::posix_time::seconds(2));
    auto outputArray = std::make_shared<std::vector<std::string>>();
    auto inputString = std::make_shared<std::string>();
    auto portSelected = std::make_shared<int>(0);
    auto boudrateString = std::make_shared<std::string>("9600");

    // control
    auto control = Container::Horizontal({
        Button(" < ", [this, connection]{
            connection->close();
            connectionSelected_ = 0;
        }),
        Container::Horizontal({
            Dropdown(&connectedPorts_, portSelected.get()) | flex,
            Input(boudrateString.get(), "boudrate")
                | CatchEvent([](Event event) { return event.is_character() && !std::isdigit(event.character()[0]); })
                | CatchEvent([](Event event) { return event == Event::Return; })
                | size(WIDTH, EQUAL, 12) | flex_shrink | border,
            Button("Open", [this, portSelected, boudrateString, connection, outputArray]{
                connection->close();
                auto port = connectedPorts_.at(*portSelected);
                auto boudrate = std::atoi(boudrateString->c_str());
                try {
                    connection->open(port, boudrate);
                    outputArray->push_back(std::format("< Connected successfully: '{}'", port));
                } catch (const boost::system::system_error& e) {
                    outputArray->push_back(std::format("< Connection error: '{}'", e.what()));
                }
            })
        }) | Maybe([this]{ return !connectedPorts_.empty(); }) | flex
    });

    // output
    auto consoleOutput = cc::ScrollableWrapper(Renderer([outputArray]{
        Elements lines;
        for (const auto& line : *outputArray) {
            lines.push_back(text(line));
        }
        return vbox(lines);
    }));

    // input
    auto inputStyle = InputOption::Default();
    inputStyle.on_enter = [connection, outputArray, inputString]{
        auto command = *inputString;
        outputArray->push_back(std::format("> {}", command));
        inputString->clear();
        
        auto output = std::string("Error: ");
        try {
            connection->writeString(command + "\n");
            output = connection->readStringUntil();
            boost::replace_all(output, "\n", "[LF]");
            boost::replace_all(output, "\r", "[CR]");
        }
        catch (const timeout_exception&) {
            output += "timeout";
        }
        catch (const boost::system::system_error&) {
            output += "disconnected";
        }
        outputArray->push_back(std::format("< {}", output));
    };
    auto consoleInput = Input(inputString.get(), inputStyle) | border;

    // result
    return Container::Vertical({
        control,
        consoleOutput,
        consoleInput
    });
}
