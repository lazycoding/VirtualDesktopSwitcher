#include <Windows.h>
#include <iostream>
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
    bool Initialize() {
        if (!settings.Load()) {
            std::cerr << "Failed to load settings" << std::endl;
            return false;
        }
        
        if (!desktopManager.Initialize()) {
            std::cerr << "Failed to initialize desktop manager" << std::endl;
            return false;
        }
        
        if (!mouseHook.Initialize([this](const MouseEvent& event) {
            return OnMouseEvent(event);
        })) {
            std::cerr << "Failed to initialize mouse hook" << std::endl;
            return false;
        }
        
        if (!overlayUI.Initialize()) {
            std::cerr << "Failed to initialize overlay UI" << std::endl;
            return false;
        }
        
        if (!trayIcon.Initialize([this]() {
            ToggleEnabled();
        })) {
            std::cerr << "Failed to initialize tray icon" << std::endl;
            return false;
        }
        
        std::cout << "Virtual Desktop Switcher initialized successfully" << std::endl;
        return true;
    }
    
    void Run() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void Shutdown() {
        mouseHook.Shutdown();
        overlayUI.Shutdown();
        trayIcon.Shutdown();
    }

private:
    bool OnMouseEvent(const MouseEvent& event) {
        if (!isEnabled) return false;
        
        // 处理鼠标事件逻辑
        return true;
    }
    
    void ToggleEnabled() {
        isEnabled = !isEnabled;
        trayIcon.UpdateState(isEnabled);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    VirtualDesktopSwitcher app;
    
    if (!app.Initialize()) {
        MessageBox(nullptr, L"Failed to initialize Virtual Desktop Switcher", L"Error", MB_ICONERROR);
        return -1;
    }
    
    app.Run();
    app.Shutdown();
    
    return 0;
}