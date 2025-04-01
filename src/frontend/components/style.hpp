#pragma once

#include <ftxui/component/component.hpp>

using namespace ftxui;

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

