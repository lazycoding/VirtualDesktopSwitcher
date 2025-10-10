#pragma once
#include "MouseHook.h"
#include "DesktopManager.h"
#include "OverlayUI.h"
#include "Settings.h"
#include "TrayIcon.h"

class VirtualDesktopSwitcher {
private:
    MouseHook mouseHook;
    DesktopManager desktopManager;
    OverlayUI overlayUI;
    Settings settings;
    TrayIcon trayIcon;
    
    bool isEnabled = true;

public:
    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool OnMouseEvent(const MouseEvent& event);
    void ToggleEnabled();
};