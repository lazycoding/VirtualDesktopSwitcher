#pragma once
#include <Windows.h>
#include <functional>
#include <string>

class TrayIcon {
public:
  using ClickCallback = std::function<void()>;

  bool Initialize(ClickCallback callback);
  void Shutdown();

  void UpdateState(bool isEnabled);
  void ShowNotification(const std::wstring &message);

private:
  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

  HWND hWnd = nullptr;
  NOTIFYICONDATA iconData = {0};
  ClickCallback callback;
  bool isEnabled = true;

  void CreateMenu();
  void UpdateIcon();
};