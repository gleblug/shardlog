#pragma once

#include "style.hpp"

#include <string>
#include <vector>
#include <functional>
#include <ftxui/component/component.hpp>

using namespace ftxui;

Component NamedMenu(const std::string& name, const std::vector<std::string>& items, int& selected, const std::unordered_map<int, std::function<void()>> &customActions = {}) {
    Components buttons;
    for (int i = 0; i < items.size(); i++) {
        std::function<void()> action = [&selected, i]{ selected = i; };
        if (customActions.contains(i)) {
            action = customActions.at(i);
        } 
        buttons.push_back(Button(items[i], action, style::buttonOptionMenu()));
    }

    auto component = Container::Vertical(buttons);

    return Renderer(component, [
        name = std::move(name),
        component
    ]{
        return vbox({
            text("  " + name) | bold,
            component->Render()
        });
    });
}
