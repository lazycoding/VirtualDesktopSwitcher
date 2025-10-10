# 虚拟桌面切换器

通过鼠标侧键滑动切换Windows虚拟桌面的工具

## 功能特性
- 鼠标侧键滑动切换桌面
- 实时显示滑动轨迹
- 系统托盘快捷控制
- 个性化配置

## 编译要求
- Windows 10/11 SDK
- CMake 3.15+
- Visual Studio 2019+

## 构建步骤
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 配置文件位置
`%APPDATA%\VirtualDesktopSwitcher\config.json`

## 图标资源
- `icon.ico` - 启用状态图标
- `disabled_icon.ico` - 禁用状态图标

## 快捷键
- 双击托盘图标: 启用/禁用
- 右键菜单: 更多选项