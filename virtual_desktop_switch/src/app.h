#pragma once

#include "DesktopManager/DesktopManager.h"
#include "GestureAnalyzer/GestureAnalyzer.h"
#include "OverlayUI/OverlayUI.h"
#include "Settings/Settings.h"
#include "TrayIcon/TrayIcon.h"
#include <memory>

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