#pragma once
#include <Windows.h>
#include <functional>
#include <string>
#include <vector>
#include <shellapi.h>

namespace VirtualDesktop {

/**
 * @brief Manages system tray icon and menu
 */
class TrayIcon {
public:
  using MenuCallback = std::function<void()>;

  TrayIcon(HINSTANCE hInstance, const std::wstring &tooltip);
  ~TrayIcon();

  bool initialize();
  void shutdown();

  void addMenuItem(const std::wstring &label, MenuCallback callback);
  void showNotification(const std::wstring &title, const std::wstring &message);

private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
  void createMenu();

  HINSTANCE m_hInstance;
  HWND m_hwnd = nullptr;
  NOTIFYICONDATAW m_notifyIconData = {};
  std::wstring m_tooltip;
  std::vector<std::pair<std::wstring, MenuCallback>> m_menuItems;
};

} // namespace VirtualDesktop
