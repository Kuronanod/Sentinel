cmake_minimum_required(VERSION 3.16)
project(Sentinel LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(UI_SRCS
    src/ui/qt.cpp
    src/ui/window.cpp
    src/ui/trafficgraph.cpp
    src/ui/topbar.cpp
    src/ui/sidebar.cpp
)

set(C_SRCS
    src/receiver/windows/get_packet.c
    src/receiver/windows/read.c
    src/receiver/windows/socket.c
    src/receiver/parser.c
    src/receiver/receiver.c
    src/receiver/packet_counter.c
)

add_executable(sentinel
    ${UI_SRCS}
    ${C_SRCS}
    include/ui/window.h
    include/ui/trafficgraph.h
    include/ui/topbar.h
    include/ui/sidebar.h
    include/receiver/windows/get_packet.h
    include/receiver/windows/read.h
    include/receiver/windows/socket.h
    include/receiver/parser.h
    include/receiver/receiver.h
    include/receiver/packet_counter.h
)

target_include_directories(sentinel PRIVATE
    include
    include/ui
    include/receiver
    include/receiver/windows
)

target_link_libraries(sentinel PRIVATE Qt6::Widgets ws2_32)