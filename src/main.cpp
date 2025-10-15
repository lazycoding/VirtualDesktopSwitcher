#include "DesktopManager/DesktopManager.h"
#include "GestureAnalyzer/GestureAnalyzer.h"
#include "MouseHook/MouseHook.h"
#include "OverlayUI/OverlayUI.h"
#include "Settings/Settings.h"
#include "TrayIcon/TrayIcon.h"
#include <Windows.h>
#include <memory>

namespace VirtualDesktop {

class Application {
public:
  Application(HINSTANCE hInstance)
      : m_hInstance(hInstance),
        m_trayIcon(hInstance, L"Virtual Desktop Switcher") {}

  bool initialize() {
    if (!m_settings.load(L"config.json")) {
      return false;
    }

    if (!m_trayIcon.initialize()) {
      return false;
    }

    m_trayIcon.addMenuItem(L"Exit", [this]() { PostQuitMessage(0); });

    if (!m_mouseHook.initialize()) {
      return false;
    }

    m_mouseHook.addCallback([this](int, WPARAM wParam, LPARAM lParam) {
      if (wParam == WM_MOUSEMOVE) {
        auto *mouseData = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
        m_gestureAnalyzer.addPosition(mouseData->pt.x, mouseData->pt.y);
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
  MouseHook &m_mouseHook = MouseHook::GetInstance();
};

} // namespace VirtualDesktop

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  VirtualDesktop::Application app(hInstance);
  if (!app.initialize()) {
    return -1;
  }
  app.run();
  return 0;
}