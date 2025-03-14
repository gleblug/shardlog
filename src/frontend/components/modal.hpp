#pragma once

#include <ftxui/component/component_options.hpp>  // for ButtonOption
#include <ftxui/component/mouse.hpp>              // for ftxui
#include <functional>                             // for function
#include <memory>                                 // for allocator, shared_ptr
 
#include "ftxui/component/component.hpp"  // for Button, operator|=, Renderer, Vertical, Modal
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive, Component
#include "ftxui/dom/elements.hpp"  // for operator|, separator, text, size, Element, vbox, border, GREATER_THAN, WIDTH, center, HEIGHT


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
