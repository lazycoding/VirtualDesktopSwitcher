cmake_minimum_required(VERSION 3.15)
project(virtual_desktop_switch LANGUAGES CXX RC)

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJ_NAME virtual_desktop_switch)

# Project warnings
if(MSVC)
    add_compile_options(/W4 /WX /wd4267 /wd4244)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-literal-operator)
endif()

# Add source files
file(GLOB_RECURSE SOURCES 
    "*.cpp"
)
file(GLOB_RECURSE HEADERS 
    "*.h" 
)
file(GLOB_RECURSE RESOURCES 
    "*.rc"
)

add_library(${PROJ_NAME} SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(${PROJ_NAME} 
PUBLIC  include
PRIVATE src third_party
)

target_compile_definitions(${PROJ_NAME} PUBLIC BUILDING_DLL)

# Windows specific libraries
if(WIN32)
    # Target Windows 10 (0x0A00) or later
    target_link_libraries(${PROJ_NAME} PRIVATE 
        user32 gdi32 d2d1 shell32 Shcore shlwapi advapi32)  # Removed gdiplus, using standard GDI
    target_compile_definitions(${PROJ_NAME} PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    )
endif()


