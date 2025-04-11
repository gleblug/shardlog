#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>  // for ButtonOption
#include <ftxui/component/mouse.hpp>              // for ftxui
#include <functional>                             // for function
#include <memory>                                 // for allocator, shared_ptr
#include <map>
#include <unordered_map>

#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive, Component
#include "ftxui/dom/elements.hpp"  // for operator|, separator, text, size, Element, vbox, border, GREATER_THAN, WIDTH, center, HEIGHT

#include "events/connection_event.hpp"
#include "events/data_event.hpp"
#include "config/config_manager.hpp"

using namespace ftxui;

namespace cc {

namespace style {
    inline ButtonOption buttonOptionMenu() {
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

inline Component ModalConfirmation(std::function<void()> yesClosure,
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

inline Component NamedMenu(const std::string& name, const std::vector<std::string>& items, int& selected, const std::unordered_map<int, std::function<void()>> &customActions = {}) {
    Components buttons;
    for (size_t i = 0; i < items.size(); i++) {
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

inline Component CollapsibleInner(std::vector<Component> children) {
    Component vlist = Container::Vertical(std::move(children));
    return Renderer(vlist, [vlist] {
        return hbox({
            text(" "),
            vlist->Render(),
        });
    });
}

inline Component ScrollableWrapper(Component content) {
    class Impl : public ComponentBase {
    private:
        float scroll_y = 1;
        Component content;
    public:
        Impl(Component inner)
        : content(inner)
        {
            auto scrollable_content = Renderer(content, [this] {
            return content->Render() | focusPositionRelative(0, scroll_y) |
                frame | flex;
            });
 
            SliderOption<float> option_y;
            option_y.value = &scroll_y;
            option_y.min = 0.f;
            option_y.max = 1.f;
            option_y.increment = 0.01f;
            option_y.direction = Direction::Down;
            auto scrollbar_y = Slider(option_y);
 
            Add(Container::Horizontal({
                scrollable_content,
                scrollbar_y,
            }) | flex );
        }
    };
    return Make<Impl>(content);
}
};
