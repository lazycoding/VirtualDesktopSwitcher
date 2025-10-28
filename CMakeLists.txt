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

# Include directories
include_directories(src third_party)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)
file(GLOB_RECURSE HEADERS 
    "src/*.h" 
)

file(GLOB_RECURSE RESOURCES 
    "src/*.rc"
)

# Main executable (WIN32 hides console)
add_executable(VirtualDesktopSwitcher WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Group files in Visual Studio solution to match directory structure
# source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${SOURCES})
# source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${RESOURCES})

# Also group by folder structure
foreach(source_file ${SOURCES} ${HEADERS} ${RESOURCES})
    get_filename_component(source_path "${source_file}" PATH)
    file(RELATIVE_PATH source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}/src" "${source_path}")
    if ("${source_path_rel}" STREQUAL "")
        set(source_path_rel "root")
    endif()
    string(REPLACE "/" "\\" source_path_msvc "${source_path_rel}")
    source_group("${source_path_msvc}" FILES "${source_file}")
endforeach()

target_include_directories(VirtualDesktopSwitcher PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR})

# Windows specific libraries
if(WIN32)
    # Target Windows 10 (0x0A00) or later
    target_link_libraries(VirtualDesktopSwitcher PRIVATE 
        user32 gdi32 d2d1 shell32 Shcore)
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


