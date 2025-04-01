#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>  // for ButtonOption
#include <ftxui/component/mouse.hpp>              // for ftxui
#include <functional>                             // for function
#include <memory>                                 // for allocator, shared_ptr
 
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive, Component
#include "ftxui/dom/elements.hpp"  // for operator|, separator, text, size, Element, vbox, border, GREATER_THAN, WIDTH, center, HEIGHT

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
};
