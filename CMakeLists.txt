cmake_minimum_required(VERSION 3.20)

project(shardlog)

find_package(xtd REQUIRED)
add_packages(spdlog)
add_references(spdlog::spdlog)

set(target_sources
    main.cpp
)
list(TRANSFORM target_sources PREPEND "src/")

add_sources(${target_sources})
target_type(CONSOLE_APPLICATION)