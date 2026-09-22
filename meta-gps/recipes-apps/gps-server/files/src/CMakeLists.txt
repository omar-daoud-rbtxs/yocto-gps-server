cmake_minimum_required(VERSION 3.10)
project(gps_server)

# Enforce modern C++ for atomics and threading
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CRITICAL: Tell CMake to automatically process Qt macros (Q_OBJECT) and resource files (.qrc)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 1. Find all required Qt modules (Added 'Widgets' for QMainWindow and QTimer)
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Network 
    Gui 
    Widgets 
    Qml 
    Quick 
    Location 
    Positioning
)

# 2. Add EVERY source and header file in your folder structure
add_executable(gps_server
    server.cpp
    backend.h
    qml.qrc
)

# 3. Link the libraries to your executable (Added Qt5::Widgets and Threads)
target_link_libraries(gps_server PRIVATE 
    Qt5::Core
    Qt5::Network
    Qt5::Gui
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    Qt5::Location
    Qt5::Positioning
    pthread
)

install(TARGETS gps_server DESTINATION bin)