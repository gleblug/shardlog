cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(shardlog)

find_package(xtd REQUIRED)
add_references(xtd.core)
add_packages(spdlog)
add_references(spdlog::spdlog_header_only)
add_packages(yaml-cpp)
add_references(yaml-cpp::yaml-cpp)

add_library(CSerialPort STATIC IMPORTED)
set_target_properties(CSerialPort PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/lib/libcserialport.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/vcpkg_installed/$ENV{VCPKG_DEFAULT_TRIPLET}/include/CSerialPort"
)
add_references(CSerialPort)

add_library(nivisa STATIC IMPORTED)
set_target_properties(nivisa PROPERTIES
    IMPORTED_LOCATION "C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/nivisa64.lib"
    INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/IVI Foundation/VISA/Win64/Include"
)
add_references(nivisa)


set(target_sources
    main.cpp
    application.cpp
    measurer/measurer.cpp
    config/config_parser.cpp
)
list(TRANSFORM target_sources PREPEND "src/")

add_sources(${target_sources})
add_compile_definitions("DEBUG=$<CONFIG:Debug>")
target_type(CONSOLE_APPLICATION)