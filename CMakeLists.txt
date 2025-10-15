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
include_directories(src third_party)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)
file(GLOB_RECURSE HEADERS 
    "src/*.h" 
)

# Main executable
add_executable(VirtualDesktopSwitcher WIN32 ${SOURCES} ${HEADERS})

# Windows specific libraries
if(WIN32)
    # Target Windows 10 (0x0A00) or later
    target_link_libraries(VirtualDesktopSwitcher PRIVATE 
        user32 gdi32 d2d1 shell32)
    target_compile_definitions(VirtualDesktopSwitcher PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
    )
endif()


