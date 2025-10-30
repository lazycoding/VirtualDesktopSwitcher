#pragma once

#include "DesktopManager/DesktopManager.h"
#include "GestureAnalyzer/GestureAnalyzer.h"
#include "OverlayUI/OverlayUI.h"
#include "Settings/Settings.h"
#include "TrayIcon/TrayIcon.h"

namespace VirtualDesktop {

class Application {
public:
    Application(HINSTANCE hInstance);
    bool initialize();
    void run();

private:
    HINSTANCE m_hInstance;
    TrayIcon m_trayIcon;
    Settings m_settings;
    DesktopManager m_desktopManager;
    GestureAnalyzer m_gestureAnalyzer;
    OverlayUI m_overlay;
};

}  // namespace VirtualDesktop