#include "app.h"
#include "MouseHook.h"
#include "utils.h"
#include <Windows.h>
#include <ShellScalingApi.h>
#include <Shlwapi.h>
#include <winreg.h>
#include <cstdlib>

// Registry key path for auto-start programs
const wchar_t* AUTO_START_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const wchar_t* APP_NAME = L"VirtualDesktopSwitcher";

namespace VirtualDesktop {

Application::Application(HINSTANCE hInstance) : m_hInstance(hInstance) {
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

    trace("Config file path: %ls", configPath.c_str());

    // Try to load existing config file
    if (!m_settings.load(configPath.c_str())) {
        // If config file doesn't exist, save default settings
        if (!m_settings.save(configPath.c_str())) {
            // If we can't save the default config, return false
            return false;
        }
    }

    // Configure auto-start based on settings
    if (m_settings.isAutoStartEnabled() != isAutoStartConfigured()) {
        setupAutoStart(m_settings.isAutoStartEnabled());
    }

    if (!m_overlay.initialize(m_hInstance)) {
        return false;
    }

    // Apply settings to overlay
    m_overlay.setSettings(m_settings);

    if (m_settings.isTrayIconEnabled()) {
        m_trayIcon = std::make_unique<TrayIcon>(m_hInstance, L"Virtual Desktop Switcher");
        if (!m_trayIcon->initialize()) {
            return false;
        }
        m_trayIcon->addMenuItem(L"Setting", []() {
            // todo: open SettingUI dialog
        });
        m_trayIcon->addMenuItem(L"Exit", []() {
            PostQuitMessage(0);
        });
    }

    MouseHook& mouseHook = MouseHook::getInstance();
    if (!mouseHook.initialize()) {
        return false;
    }

    // Since lambda in member functions can't capture 'this' directly in initialization,
    // we create a local copy of the callback to use with the mouse hook
    auto callback = [this](int code, WPARAM wParam, LPARAM lParam) {
        UNREFERENCED_PARAMETER(code);
        if (m_settings.getTriggerButton() == MouseButton::None) {
            trace("Trigger button is set to None, ignoring mouse events.");
            return;
        }

        // Handle mouse events based on configured trigger button
        int triggerButton = static_cast<int>(m_settings.getTriggerButton());
        if (wParam == WM_XBUTTONDOWN || wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            bool isTriggerButton = (wParam == WM_XBUTTONDOWN) ? (HIWORD(mouseData->mouseData) == (triggerButton - 2))
                                                              : ((int)(wParam - WM_LBUTTONDOWN + 1) == triggerButton);
            if (!isTriggerButton) {
                return;
            }
            // Start gesture analysis
            m_gestureAnalyzer.clearPositions();
            m_gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
            m_overlay.show();
            m_overlay.updatePosition(mouseData->pt.x, mouseData->pt.y);
        } else if (wParam == WM_XBUTTONUP || wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP) {
            auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            bool isTriggerButton = (wParam == WM_XBUTTONUP) ? (HIWORD(mouseData->mouseData) == (triggerButton - 2))
                                                            : ((int)(wParam - WM_LBUTTONUP + 1) == triggerButton);
            if (!isTriggerButton) {
                return;
            }
            // Analyze gesture and switch virtual desktop
            auto direction = m_gestureAnalyzer.analyzeGesture();
            trace("Gesture direction: %d", static_cast<int>(direction));
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

bool Application::setupAutoStart(bool enable) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, AUTO_START_KEY, 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    if (enable) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        result = RegSetValueExW(
                hKey, APP_NAME, 0, REG_SZ, (const BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t));
    } else {
        result = RegDeleteValueW(hKey, APP_NAME);
    }

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool Application::isAutoStartConfigured() const {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, AUTO_START_KEY, 0, KEY_QUERY_VALUE, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    result = RegQueryValueExW(hKey, APP_NAME, NULL, NULL, NULL, NULL);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

}  // namespace VirtualDesktop