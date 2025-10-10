#include "TrayIcon.h"
#include <shellapi.h>

// 静态成员初始化
HWND TrayIcon::hWnd = nullptr;
NOTIFYICONDATA TrayIcon::iconData = {0};
TrayIcon::ClickCallback TrayIcon::callback = nullptr;
bool TrayIcon::isEnabled = true;

bool TrayIcon::Initialize(ClickCallback cb) {
  callback = cb;

  // 创建隐藏窗口
  WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = L"VirtualDesktopSwitcherTrayIcon";

  if (!RegisterClassEx(&wc)) {
    return false;
  }

  hWnd = CreateWindowEx(0, wc.lpszClassName, nullptr, 0, 0, 0, 0, 0,
                        HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), this);

  if (!hWnd) {
    return false;
  }

  // 初始化托盘图标
  iconData.cbSize = sizeof(NOTIFYICONDATA);
  iconData.hWnd = hWnd;
  iconData.uID = 1;
  iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
  iconData.uCallbackMessage = WM_APP + 1;
  iconData.hIcon =
      LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(1)); // 使用应用图标

  wcscpy_s(iconData.szTip, L"虚拟桌面切换器");

  if (!Shell_NotifyIcon(NIM_ADD, &iconData)) {
    DestroyWindow(hWnd);
    hWnd = nullptr;
    return false;
  }

  // 设置新版托盘图标
  iconData.uVersion = NOTIFYICON_VERSION_4;
  Shell_NotifyIcon(NIM_SETVERSION, &iconData);

  return true;
}

void TrayIcon::Shutdown() {
  if (hWnd) {
    Shell_NotifyIcon(NIM_DELETE, &iconData);
    DestroyWindow(hWnd);
    hWnd = nullptr;
  }
  callback = nullptr;
}

void TrayIcon::UpdateState(bool enabled) {
  isEnabled = enabled;
  UpdateIcon();
}

void TrayIcon::ShowNotification(const std::wstring &message) {
  iconData.uFlags = NIF_INFO;
  wcscpy_s(iconData.szInfo, message.c_str());
  wcscpy_s(iconData.szInfoTitle, L"虚拟桌面切换器");
  iconData.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
  Shell_NotifyIcon(NIM_MODIFY, &iconData);
}

LRESULT CALLBACK TrayIcon::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam) {
  if (uMsg == WM_APP + 1) {
    switch (LOWORD(lParam)) {
    case WM_CONTEXTMENU:
      reinterpret_cast<TrayIcon *>(GetWindowLongPtr(hWnd, GWLP_USERDATA))
          ->CreateMenu();
      break;

    case WM_LBUTTONDBLCLK:
      if (callback) {
        callback();
      }
      break;
    }
  } else if (uMsg == WM_COMMAND) {
    if (LOWORD(wParam) == 1 && callback) {
      callback();
    } else if (LOWORD(wParam) == 2) {
      PostQuitMessage(0);
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void TrayIcon::CreateMenu() {
  HMENU hMenu = CreatePopupMenu();
  if (hMenu) {
    AppendMenu(hMenu, MF_STRING, 1, isEnabled ? L"禁用" : L"启用");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, 2, L"退出");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd,
                   nullptr);
    PostMessage(hWnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
  }
}

void TrayIcon::UpdateIcon() {
  iconData.hIcon =
      LoadIcon(GetModuleHandle(nullptr),
               MAKEINTRESOURCE(isEnabled ? 1 : 2)); // 使用不同状态图标
  Shell_NotifyIcon(NIM_MODIFY, &iconData);
}