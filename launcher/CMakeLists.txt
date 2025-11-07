cmake_minimum_required(VERSION 3.15)
project(VirtualDesktopSwitcher LANGUAGES CXX RC)

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

# Main executable (WIN32 hides console)
add_executable(VirtualDesktopSwitcher WIN32 main.cpp)

add_subdirectory(../virtual_desktop_switch virtual_desktop_switch)

# Include directories
target_include_directories(VirtualDesktopSwitcher PRIVATE
 ../virtual_desktop_switch/include
)

# Target Windows 10 (0x0A00) or later
target_link_libraries(VirtualDesktopSwitcher PRIVATE 
        user32 gdi32 d2d1 shell32 Shcore virtual_desktop_switch)  # Removed gdiplus, using standard GDI
target_compile_definitions(VirtualDesktopSwitcher PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
)

# Copy vds.dll to executable directory after build
add_custom_command(TARGET VirtualDesktopSwitcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:virtual_desktop_switch>
    $<TARGET_FILE_DIR:VirtualDesktopSwitcher>
    COMMENT "Copying virtual_desktop_switch.dll to executable directory"
)