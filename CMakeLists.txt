cmake_minimum_required(VERSION 3.16)
project(Sentinel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(UI_SRCS
    src/ui/qt.cpp
    src/ui/window.cpp
    src/ui/trafficgraph.cpp
    src/ui/topbar.cpp
)

add_executable(sentinel
    ${UI_SRCS}
    include/ui/window.h
    include/ui/trafficgraph.h
    include/ui/topbar.h
)

target_include_directories(sentinel PRIVATE
    include
    include/ui
)

target_link_libraries(sentinel PRIVATE Qt6::Widgets)