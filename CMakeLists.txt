cmake_minimum_required(VERSION 3.15)
project(VirtualDesktopSwitcher LANGUAGES CXX)

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories
include_directories(src)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Main executable
add_executable(VirtualDesktopSwitcher ${SOURCES} ${HEADERS})

# Windows specific settings
if(WIN32)
    target_link_libraries(VirtualDesktopSwitcher PRIVATE user32 gdi32 d2d1)
    target_compile_definitions(VirtualDesktopSwitcher PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# Testing configuration
enable_testing()
find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
add_executable(VirtualDesktopSwitcherTests ${TEST_SOURCES} ${SOURCES} ${HEADERS})
target_link_libraries(VirtualDesktopSwitcherTests PRIVATE GTest::GTest GTest::Main)
if(WIN32)
    target_link_libraries(VirtualDesktopSwitcherTests PRIVATE user32 gdi32 d2d1)
endif()

add_test(NAME UnitTests COMMAND VirtualDesktopSwitcherTests)

# Integration tests
add_executable(VirtualDesktopSwitcherIntegrationTests 
    test/Integration/SystemIntegrationTest.cpp
    ${SOURCES} ${HEADERS})
target_link_libraries(VirtualDesktopSwitcherIntegrationTests 
    PRIVATE GTest::GTest GTest::Main)
if(WIN32)
    target_link_libraries(VirtualDesktopSwitcherIntegrationTests 
        PRIVATE user32 gdi32 d2d1)
endif()

add_test(NAME IntegrationTests COMMAND VirtualDesktopSwitcherIntegrationTests)