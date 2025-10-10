#define NOMINMAX
#include "../include/VirtualDesktopSwitcher.h"
#include "../include/DesktopManager.h"
#include "../include/MouseHook.h"
#include "../include/OverlayUI.h"
#include "../include/Settings.h"
#include "../include/TrayIcon.h"
#include <Windows.h>
#include <iostream>

bool VirtualDesktopSwitcher::Initialize() {
  if (!settings.Load()) {
    std::cerr << "Failed to load settings" << std::endl;
    return false;
  }

  if (!desktopManager.Initialize()) {
    std::cerr << "Failed to initialize desktop manager" << std::endl;
    return false;
  }

  if (!mouseHook.Initialize(
          [this](const MouseEvent &event) { return OnMouseEvent(event); })) {
    std::cerr << "Failed to initialize mouse hook" << std::endl;
    return false;
  }

  if (!overlayUI.Initialize()) {
    std::cerr << "Failed to initialize overlay UI" << std::endl;
    return false;
  }

  if (!trayIcon.Initialize([this]() { ToggleEnabled(); })) {
    std::cerr << "Failed to initialize tray icon" << std::endl;
    return false;
  }

  std::cout << "Virtual Desktop Switcher initialized successfully" << std::endl;
  return true;
}

void VirtualDesktopSwitcher::Run() {
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void VirtualDesktopSwitcher::Shutdown() {
  mouseHook.Shutdown();
  overlayUI.Shutdown();
  trayIcon.Shutdown();
}

bool VirtualDesktopSwitcher::OnMouseEvent(const MouseEvent &event) {
  if (!isEnabled)
    return false;

  // 处理鼠标事件逻辑
  return true;
}

void VirtualDesktopSwitcher::ToggleEnabled() {
  isEnabled = !isEnabled;
  trayIcon.UpdateState(isEnabled);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  VirtualDesktopSwitcher app;

  if (!app.Initialize()) {
    MessageBoxW(nullptr, L"Failed to initialize Virtual Desktop Switcher",
                L"Error", MB_ICONERROR);
    return -1;
  }

  app.Run();
  app.Shutdown();

  return 0;
}