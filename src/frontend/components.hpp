#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>  // for ButtonOption
#include <ftxui/component/mouse.hpp>              // for ftxui
#include <functional>                             // for function
#include <memory>                                 // for allocator, shared_ptr
 
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive, Component
#include "ftxui/dom/elements.hpp"  // for operator|, separator, text, size, Element, vbox, border, GREATER_THAN, WIDTH, center, HEIGHT

#include "events/connection_event.hpp"
#include "events/data_event.hpp"
#include "config/config_manager.hpp"

using namespace ftxui;

namespace cc {

namespace style {
    ButtonOption buttonOptionMenu() {
        ButtonOption buttonOption;
        buttonOption.transform = [](EntryState state) {
            state.label = (state.focused ? "> " : "  ") + state.label;
            Element e = text(state.label) | border | size(HEIGHT, EQUAL, 3);
            if (state.active)
                e |= bold;
            return e;
        };
        return buttonOption;
    }
}

Component ModalConfirmation(std::function<void()> yesClosure,
                         std::function<void()> noClosure) {
  auto component = Container::Horizontal({
      Button("Yes", yesClosure) | flex,
      Button("No", noClosure) | flex,
  });
  component |= Renderer([&](Element inner) {
    return vbox({
               text("Are you sure?") | bold | center,
               separator(),
               inner,
           })
           | size(WIDTH, GREATER_THAN, 30)
           | border;
  });
  return component;
}

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

Component CollapsibleInner(std::vector<Component> children) {
    Component vlist = Container::Vertical(std::move(children));
    return Renderer(vlist, [vlist] {
        return hbox({
            text(" "),
            vlist->Render(),
        });
    });
}

Component DevicesResultComponent(const DevicesResult& results) {
    return Renderer([&results] {
        Elements elements;
        for (const auto& [name, result] : results) {
            Elements resElements;
            for (const auto& [title, value] : result.values) {
                resElements.push_back(text(title + ": " + value));
            }

            std::string status;
            switch (result.status) {
                case MeasurementStatus::READY:
                    status = "Ready";
                    break;
                case MeasurementStatus::TIMEOUT:
                    status = "Timeout";
                    break;
                case MeasurementStatus::DISCONNECTED:
                    status = "Disconnected";
                    break;
                default:
                    status = "Unknown";
                    break;
            }

            auto devElement = hbox({
                text(name) | flex,
                text(status) | bold
            });

            if (!resElements.empty()) {
                devElement = vbox({
                    devElement,
                    separator(),
                    vbox(resElements)
                });
            }
            elements.push_back(devElement | border);
        }
        return vbox(elements);
    });
}
};
