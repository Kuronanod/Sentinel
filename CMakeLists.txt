cmake_minimum_required(VERSION 3.16)
project(Sentinel LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(UI_SRCS
    src/ui/qt.cpp
    src/ui/window.cpp
    src/ui/trafficgraph.cpp
    src/ui/topbar.cpp
    src/ui/sidebar.cpp
    src/ui/terminal.cpp
    src/ui/packetpage.cpp
    src/ui/alertpage.cpp
    src/ui/managementpage.cpp
)

set(CORE_SRCS
    src/receiver/parser.c
    src/receiver/receiver.c
    src/receiver/packet_counter.c
    src/receiver/request_queue.cpp
    src/receiver/prefilter.c
    src/receiver/alert_queue.cpp
    src/receiver/firewall.c
)

if(WIN32)
    set(OS_SRCS
        src/receiver/windows/socket.c
        src/receiver/windows/get_packet.c
        src/receiver/windows/read.c
    )
    set(OS_LIBS ws2_32)
    set(OS_HEADERS
        include/receiver/windows/socket.h
        include/receiver/windows/get_packet.h
        include/receiver/windows/read.h
    )
elseif(UNIX AND NOT APPLE)
    set(OS_SRCS
        src/receiver/linux/socket.c
        src/receiver/linux/ring_buffer.c
        src/receiver/linux/read.c
    )
    set(OS_LIBS)
    set(OS_HEADERS
        include/receiver/linux/socket.h
        include/receiver/linux/ring_buffer.h
        include/receiver/linux/read.h
    )
else()
    message(FATAL_ERROR "OS not supported")
endif()

add_executable(sentinel
    ${UI_SRCS}
    ${CORE_SRCS}
    ${OS_SRCS}
    include/ui/window.h
    include/ui/trafficgraph.h
    include/ui/topbar.h
    include/ui/sidebar.h
    include/ui/terminal.h
    include/ui/packetpage.h
    include/ui/alertpage.h
    include/ui/managementpage.h
    include/receiver/receiver.h
    include/receiver/parser.h
    include/receiver/packet_counter.h
    include/receiver/request_queue.h
    include/receiver/alert_queue.h
    include/receiver/prefilter.h
    include/receiver/firewall.h
    ${OS_HEADERS}
)

target_include_directories(sentinel PRIVATE
    include
    include/ui
    include/receiver
    include/receiver/windows
    include/receiver/linux
)

if(WIN32)
    target_link_libraries(sentinel PRIVATE Qt6::Widgets Qt6::Network ${OS_LIBS})
else()
    target_link_libraries(sentinel PRIVATE Qt6::Widgets Qt6::Network ${OS_LIBS})
endif()