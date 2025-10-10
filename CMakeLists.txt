cmake_minimum_required(VERSION 3.15)
project(VirtualDesktopSwitcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(nlohmann_json REQUIRED)

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
add_executable(VirtualDesktopSwitcher
    VirtualDesktopSwitcher.cpp
    MouseHook.cpp
    DesktopManager.cpp
    OverlayUI.cpp
    Settings.cpp
    TrayIcon.cpp
    TestRunner.cpp
)

target_include_directories(VirtualDesktopSwitcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VirtualDesktopSwitcher PRIVATE ${PLATFORM_LIBS} nlohmann_json::nlohmann_json)

# 安装目标
install(TARGETS VirtualDesktopSwitcher
    RUNTIME DESTINATION bin
)

# 资源文件
if(WIN32)
    # 添加资源文件
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
        COMMAND echo "1 ICON \"icon.ico\"" > ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
        COMMAND echo "2 ICON \"disabled_icon.ico\"" >> ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
    )
    
    add_custom_target(Resources DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
    add_dependencies(VirtualDesktopSwitcher Resources)
    
    target_sources(VirtualDesktopSwitcher PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
endif()