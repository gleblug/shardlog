cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(shardlog LANGUAGES CXX VERSION 1.0.0)

add_compile_definitions("DEBUG=$<CONFIG:Debug>")
add_compile_definitions("PROJECT_NAME=\"${PROJECT_NAME}\"")

include_directories(src)
set(target_sources
    main.cpp
    application.cpp
    frontend/console_frontend.cpp
    connection/serial.cpp
    connection/connection_manager.cpp
    # config/config_parser.cpp
    # hardware/serial/ListSerial.cpp
    # application.cpp
    # measurer/measurer.cpp
    # config/config_parser.cpp
    # meter/meter.cpp
    # meter/connection/connection.cpp
    # meter/connection/comport.cpp
    # meter/connection/nivisa.cpp
)
list(TRANSFORM target_sources PREPEND "src/")

add_executable(${PROJECT_NAME} ${target_sources})

find_package(boost_asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::asio)

find_package(ftxui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ftxui::dom ftxui::screen ftxui::component)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp::yaml-cpp)

find_path(PULZED_MINI_INCLUDE_DIRS "mini/ini.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${PULZED_MINI_INCLUDE_DIRS})
