cmake_minimum_required(VERSION 3.20)

project(shardlog)

find_package(xtd REQUIRED)
add_packages(spdlog)
add_references(spdlog::spdlog_header_only)

add_library(nivisa STATIC IMPORTED)
set_target_properties(nivisa PROPERTIES
    IMPORTED_LOCATION "C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/nivisa64.lib"
    INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/IVI Foundation/VISA/Win64/Include"
)
add_references(nivisa)


set(target_sources
    main.cpp
)
list(TRANSFORM target_sources PREPEND "src/")

add_sources(${target_sources})
target_type(CONSOLE_APPLICATION)