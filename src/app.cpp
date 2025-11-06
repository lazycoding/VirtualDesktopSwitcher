#include "app.h"
#include "MouseHook/MouseHook.h"
#include "utils.h"
#include <Windows.h>
#include <ShellScalingApi.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace VirtualDesktop {

Application::Application(HINSTANCE hInstance) :
        m_hInstance(hInstance), m_trayIcon(hInstance, L"Virtual Desktop Switcher") {
}

bool Application::initialize() {
    // Use more compatible way to set DPI awareness
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    // Construct config file path relative to executable location
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    // Remove filename to keep only directory
    PathRemoveFileSpecW(exePath);
    
    // Combine directory with config filename
    std::wstring configPath = std::wstring(exePath) + L"\\config.json";
    
    // Try to load existing config file
    if (!m_settings.load(configPath.c_str())) {
        // If config file doesn't exist, save default settings
        if (!m_settings.save(configPath.c_str())) {
            // If we can't save the default config, return false
            return false;
        }
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

    MouseHook& mouseHook = MouseHook::getInstance();
    if (!mouseHook.initialize()) {
        return false;
    }

    // Since lambda in member functions can't capture 'this' directly in initialization,
    // we create a local copy of the callback to use with the mouse hook
    auto callback = [this](int code, WPARAM wParam, LPARAM lParam) {
        UNREFERENCED_PARAMETER(code);
        // Mouse side button pressed
        if (wParam == WM_XBUTTONDOWN) {
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            bool isBackButton = (HIWORD(mouseData->mouseData) == XBUTTON1);
            if (!isBackButton) {
                return;
            }
            // Start gesture analysis
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
            // Analyze gesture and switch virtual desktop
            auto direction = m_gestureAnalyzer.analyzeGesture();
            trace("Gesture direction: %d\n", static_cast<int>(direction));
            if (direction == GestureAnalyzer::Direction::Left) {
                m_desktopManager.switchDesktop(false);
            } else if (direction == GestureAnalyzer::Direction::Right) {
                m_desktopManager.switchDesktop(true);
            }
            m_gestureAnalyzer.clearPositions();
            m_overlay.hide();
        } else if (wParam == WM_MOUSEMOVE) {
            // Record mouse movement only when side button is pressed
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