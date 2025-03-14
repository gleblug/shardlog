#pragma once

#include "config/config_parser.hpp"

#include <ftxui/component/component.hpp>

using namespace ftxui;

// Component DeviceList() {
//     auto names = ConfigParser::deviceNames();
//     return Renderer([=]{
//         Elements entries;
//         for (auto & name : names) {
//             entries.push_back(
//                 text(name) | border
//             );
//         }
//         return vbox(entries);
//     });
// };
