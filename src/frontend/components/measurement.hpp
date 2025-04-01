#pragma once

#include "config/config_manager.hpp"
#include <ftxui/component/component.hpp>

using namespace ftxui;

Component MeasurementList() {
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
};
