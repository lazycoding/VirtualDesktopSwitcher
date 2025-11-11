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

# === 统一产物输出目录 ===
# 二进制（exe/dll/so/dylib）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 静态/动态库（.lib / .a / .dll / .so）
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 对象文件（.obj / .o）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 保证多配置生成器（Visual Studio / Xcode）下也一致
foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

include_directories(third_party)

add_subdirectory(core)
add_subdirectory(app)