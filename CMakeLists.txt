cmake_minimum_required(VERSION 3.10)
project(GUIPlayer VERSION 1.0)

# Find packages
find_package(PkgConfig REQUIRED)

# GStreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0)

# GTK3
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Add executable
add_executable(gui-player 
    src/main.cpp
    src/PlayerGUI.cpp
)

# Include directories
target_include_directories(gui-player PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(gui-player
    ${GSTREAMER_LIBRARIES}
    ${GTK3_LIBRARIES}
)

# C++ standard
set_target_properties(gui-player PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Set C++ flags
target_compile_options(gui-player PRIVATE -std=c++17)