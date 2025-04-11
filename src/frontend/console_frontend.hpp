#pragma once

#include <unordered_map>
#include <vector>
#include <chrono>

#include <ftxui/component/screen_interactive.hpp>

#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"
#include "events/command_event.hpp"
#include "events/data_event.hpp"

using namespace ftxui;
namespace chrono = std::chrono;
using namespace std::chrono_literals;

class ConsoleFrontend {
public:
    ConsoleFrontend(CommandBus commandBus);
    void run();
    void handleConnection(const ConnectionEvent& event);
    void handleData(const DataEvent& event);
    
private:
    CommandBus commandBus_;
    ScreenInteractive screen_;

    std::vector<std::string> connectedPorts_;
    std::unordered_map<std::string, ConnectionType> portsStatus_;
    
    void activateMeasurement(const std::string& experimentName, const std::string& measurementName);

    Component Measurements();
    Component Desk();
    int measurementsSelected_ = 0;
    bool measuring_ = false;
    std::map<std::string, DeviceData> devicesData_;
    chrono::seconds remainS_ = 0s;
    float percentage_ = 0.0;

    Component Connections();
    Component Terminal();
    int connectionSelected_ = 0;
};
