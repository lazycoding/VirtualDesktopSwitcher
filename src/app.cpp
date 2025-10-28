#include "app.h"
#include "MouseHook/MouseHook.h"

#include <Windows.h>
#include <ShellScalingApi.h>

namespace VirtualDesktop {

Application::Application(HINSTANCE hInstance) :
        m_hInstance(hInstance), m_trayIcon(hInstance, L"Virtual Desktop Switcher") {
}

bool Application::initialize() {
    // 使用更兼容的方式设置DPI感知
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    if (!m_settings.load(L"config.json")) {
        return false;
    }

    if (!m_overlay.initialize(m_hInstance)) {
        return false;
    }

    // Apply settings to overlay
    m_overlay.setSettings(m_settings);

    if (!m_trayIcon.initialize()) {
        return false;
    }

    m_trayIcon.addMenuItem(L"Setting", []() {
        // todo: open SettingUI dialog
    });
    m_trayIcon.addMenuItem(L"Exit", []() {
        PostQuitMessage(0);
    });

    MouseHook& mouseHook = MouseHook::GetInstance();
    if (!mouseHook.initialize()) {
        return false;
    }

    // Since lambda in member functions can't capture 'this' directly in initialization,
    // we create a local copy of the callback to use with the mouse hook
    auto callback = [this](int code, WPARAM wParam, LPARAM lParam) {
        UNREFERENCED_PARAMETER(code);
        // 鼠标侧键按下
        if (wParam == WM_XBUTTONDOWN) {
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            bool isBackButton = (HIWORD(mouseData->mouseData) == XBUTTON1);
            if (!isBackButton) {
                return;
            }
            // 开始分析手势
            m_gestureAnalyzer.clearPositions();
            m_gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
            m_overlay.show();
            m_overlay.updatePosition(mouseData->pt.x, mouseData->pt.y);
        } else if (wParam == WM_XBUTTONUP) {
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            bool isBackButton = (HIWORD(mouseData->mouseData) == XBUTTON1);
            if (!isBackButton) {
                return;
            }
            // 分析手势并切换虚拟桌面
            auto direction = m_gestureAnalyzer.analyzeGesture();
            if (direction == GestureAnalyzer::Direction::Left) {
                m_desktopManager.switchDesktop(false);
            } else if (direction == GestureAnalyzer::Direction::Right) {
                m_desktopManager.switchDesktop(true);
            }
            m_gestureAnalyzer.clearPositions();
            m_overlay.hide();
        } else if (wParam == WM_MOUSEMOVE) {
            // 记录鼠标移动位置仅当侧键按下时
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            if (m_gestureAnalyzer.isGestureInProgress()) {
                m_gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
                m_overlay.updatePosition(mouseData->pt.x, mouseData->pt.y);
            }
        }
    };

    mouseHook.addCallback(callback);

    return true;
}

void Application::run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    m_settings.save(L"config.json");
}

}  // namespace VirtualDesktop