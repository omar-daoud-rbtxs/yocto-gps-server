cmake_minimum_required(VERSION 3.10)

# The project name becomes the default name of the binary in Yocto
project(GpsMockClient VERSION 1.0 LANGUAGES CXX)

# Use modern C++ (C++17 is standard for most embedded environments now)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find standard threading libraries (needed if you use std::thread for your loop)
find_package(Threads REQUIRED)

# Define your executable and its source files
add_executable(gps_client 
    client.cpp
    # Add other files like ClientSocket.cpp here later if you split them up
)

# Link the threading library to your executable
target_link_libraries(gps_client PRIVATE Threads::Threads)

# This install rule is CRITICAL for Yocto. 
# It tells Yocto where to put the binary on the Pi's file system (e.g., /usr/bin)
install(TARGETS gps_client DESTINATION bin)