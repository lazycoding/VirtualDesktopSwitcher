#include "DesktopManager/DesktopManager.h"
#include "GestureAnalyzer/GestureAnalyzer.h"
#include "MouseHook/MouseHook.h"
#include "OverlayUI/OverlayUI.h"
#include "Settings/Settings.h"
#include "TrayIcon/TrayIcon.h"
#include <Windows.h>
#include <spdlog/spdlog.h>

namespace VirtualDesktop {

    class Application {
    public:
        Application(HINSTANCE hInstance)
            : m_hInstance(hInstance),
            m_trayIcon(hInstance, L"Virtual Desktop Switcher") {
        }

        bool initialize() {
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

            m_trayIcon.addMenuItem(L"Exit", [this]() { PostQuitMessage(0); });

            if (!m_mouseHook.initialize()) {
                return false;
            }

            m_mouseHook.addCallback([this](int, WPARAM wParam, LPARAM lParam) {
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
                }
                else if (wParam == WM_XBUTTONUP) {
                    auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
                    bool isBackButton = (HIWORD(mouseData->mouseData) == XBUTTON1);
                    if (!isBackButton) {
                        return;
                    }
                    // 分析手势并切换虚拟桌面
                    auto direction = m_gestureAnalyzer.analyzeGesture();
                    if (direction == GestureAnalyzer::Direction::Left) {
                        m_desktopManager.switchDesktop(false);
                        //m_trayIcon.showNotification(L"Switched Desktop",
                        //                            L"Moved to previous virtual desktop.");
                    }
                    else if (direction == GestureAnalyzer::Direction::Right) {
                        m_desktopManager.switchDesktop(true);
                        //m_trayIcon.showNotification(L"Switched Desktop",
                        //                            L"Moved to next virtual desktop.");
                    }
                    m_gestureAnalyzer.clearPositions();
                    m_overlay.hide();
                }
                else if (wParam == WM_MOUSEMOVE) {
                    // 记录鼠标移动位置仅当侧键按下时
                    auto* mouseData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
                    if (m_gestureAnalyzer.isGestureInProgress()) {
                        m_gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
                        m_overlay.updatePosition(mouseData->pt.x, mouseData->pt.y);
                    }
                }
                });

            return true;
        }

        void run() {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

    private:
        HINSTANCE m_hInstance;
        TrayIcon m_trayIcon;
        Settings m_settings;
        DesktopManager m_desktopManager;
        GestureAnalyzer m_gestureAnalyzer;
        OverlayUI m_overlay;
        MouseHook& m_mouseHook = MouseHook::GetInstance();
    };

} // namespace VirtualDesktop

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    VirtualDesktop::Application app(hInstance);
    if (!app.initialize()) {
        return -1;
    }
    app.run();
    return 0;
}