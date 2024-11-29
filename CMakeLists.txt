cmake_minimum_required(VERSION 3.20)

project(shardlog)

find_package(xtd REQUIRED)
add_packages(spdlog)
add_references(spdlog::spdlog)
add_packages(PkgConfig)
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
add_references(PkgConfig::libusb)

set(target_sources
    main.cpp
)
list(TRANSFORM target_sources PREPEND "src/")

add_sources(${target_sources})
target_type(CONSOLE_APPLICATION)