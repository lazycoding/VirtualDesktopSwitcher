cmake_minimum_required(VERSION 3.15)
project(VirtualDesktopSwitcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 包含Direct2D和COM相关库
if(WIN32)
    set(PLATFORM_LIBS
        d2d1.lib
        dwrite.lib
        windowsapp.lib
        ole32.lib
        shell32.lib
        shlwapi.lib
    )
endif()

# 添加可执行文件
add_executable(VirtualDesktopSwitcher WIN32
    src/VirtualDesktopSwitcher.cpp
    src/MouseHook.cpp
    src/DesktopManager.cpp
    src/OverlayUI.cpp
    src/Settings.cpp
    src/TrayIcon.cpp
)

# 手动下载json.hpp到third_party目录
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json.hpp")
    file(DOWNLOAD 
        https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json.hpp"
        SHOW_PROGRESS
    )
endif()

# 包含目录
target_include_directories(VirtualDesktopSwitcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} 
third_party include)
target_link_libraries(VirtualDesktopSwitcher PRIVATE ${PLATFORM_LIBS})
target_compile_definitions(VirtualDesktopSwitcher PRIVATE UNICODE)
# 安装目标
install(TARGETS VirtualDesktopSwitcher
    RUNTIME DESTINATION bin
)

# 资源文件
if(WIN32)
    # 添加资源文件
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
        COMMAND echo "1 ICON \"../resources/app.ico\"" > ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
        COMMAND echo "2 ICON \"../resources/app_dis.ico\"" >> ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
    )
    
    add_custom_target(Resources DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
    add_dependencies(VirtualDesktopSwitcher Resources)
    
    target_sources(VirtualDesktopSwitcher PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
endif()