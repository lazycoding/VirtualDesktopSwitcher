cmake_minimum_required(VERSION 3.15)
project(VirtualDesktopSwitcher LANGUAGES CXX RC)

# Include FetchContent module
include(FetchContent)

# Download and install spdlog
# FetchContent_Declare(
#     spdlog
#     GIT_REPOSITORY https://github.com/gabime/spdlog.git
#     GIT_TAG v1.13.0
# )
# FetchContent_MakeAvailable(spdlog)

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project warnings
if(MSVC)
    add_compile_options(/W4 /WX /wd4267)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-literal-operator)
endif()

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)
file(GLOB_RECURSE HEADERS 
    "src/*.h" 
    "include/*.h"
)
file(GLOB_RECURSE RESOURCES 
    "src/*.rc"
)

# Main executable (WIN32 hides console)
add_executable(VirtualDesktopSwitcher WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(VirtualDesktopSwitcher PRIVATE 
include
src
third_party
)

# Windows specific libraries
if(WIN32)
    # Target Windows 10 (0x0A00) or later
    target_link_libraries(VirtualDesktopSwitcher PRIVATE 
        user32 gdi32 d2d1 shell32 Shcore)  # Removed gdiplus, using standard GDI
    target_compile_definitions(VirtualDesktopSwitcher PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    )
endif()


