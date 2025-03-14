#include "devices_status.hpp"

#include <spdlog/spdlog.h>

namespace lg = spdlog;

void DevicesStatus::addPort(const std::string& port, ConnectionEvent::Type type) {
    usedPorts_[port] = type;
}

void DevicesStatus::removePort(const std::string& port) {
    if (!usedPorts_.contains(port)) {
        lg::warn("Device disconnected while not specify in status component: {}", port);
    } else {
        usedPorts_.erase(port);
    }
}

Component DevicesStatus::componentReady() {
    return Renderer([this]{
        Elements ports;
        for (const auto& [port, status] : usedPorts_) {
            Color status_color;
            std::string status_str;
            switch (status)
            {
            case ConnectionEvent::Type::CONNECTED:
                continue;
            case ConnectionEvent::Type::READY:
                status_color = Color::GreenLight;
                status_str = "Ready";
                break;
            case ConnectionEvent::Type::TIMEOUT:
                status_color = Color::YellowLight;
                status_str = "Timeout";
                break;
            case ConnectionEvent::Type::DISCONNECTED:
                status_color = Color::RedLight;
                status_str = "Disconnected";
                break;
            default:
                break;
            }
            ports.push_back(vbox({
                hbox(
                    text(" ●  ") | color(status_color), 
                    text(port)
                ) | bold,
                separator(),
                text(status_str),
            }) | border | size(WIDTH, EQUAL, 20));
        }
        return flexbox(ports);
    });
}

Component DevicesStatus::componentConnected()
{
    return Renderer([this]{
        Elements ports;
        for (const auto& [port, status] : usedPorts_) {
            if (status != ConnectionEvent::Type::CONNECTED) {
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
