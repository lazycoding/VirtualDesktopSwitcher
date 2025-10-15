#pragma once
#include <Windows.h>
#include <functional>
#include <vector>

namespace VirtualDesktop {

/**
 * @brief Captures and processes mouse events using Windows hook
 */
class MouseHook {
public:
  using EventCallback = std::function<void(int, WPARAM, LPARAM)>;

  static MouseHook &GetInstance();

  bool initialize();
  void shutdown();

  void addCallback(const EventCallback &callback);
  void removeCallbacks();

private:
  MouseHook() = default;
  ~MouseHook() = default;

  static LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam);

  HHOOK m_hook = nullptr;
  std::vector<EventCallback> m_callbacks;
};

} // namespace VirtualDesktop