#pragma once

#include "DesktopManager.h"
#include "GestureAnalyzer.h"
#include "OverlayUI.h"
#include "Settings.h"
#include "TrayIcon.h"

namespace VirtualDesktop {

class Application {
public:
    Application(HINSTANCE hInstance);
    bool initialize();
    void run();

private:
    bool setupAutoStart(bool enable);
    bool isAutoStartConfigured() const;

    HINSTANCE m_hInstance;
    std::unique_ptr<TrayIcon> m_trayIcon;
    Settings m_settings;
    DesktopManager m_desktopManager;
    GestureAnalyzer m_gestureAnalyzer;
    OverlayUI m_overlay;
};

}  // namespace VirtualDesktop