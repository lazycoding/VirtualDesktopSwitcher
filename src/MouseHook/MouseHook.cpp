#include "MouseHook.h"
#include <stdexcept>

namespace VirtualDesktop {

MouseHook &MouseHook::GetInstance() {
  static MouseHook instance;
  return instance;
}

bool MouseHook::initialize() {
  if (m_hook != nullptr) {
    return true;
  }

  m_hook = SetWindowsHookEx(WH_MOUSE_LL, HookCallback, nullptr, 0);
  if (m_hook == nullptr) {
    throw std::runtime_error("Failed to set mouse hook");
  }
  return true;
}

void MouseHook::shutdown() {
  if (m_hook != nullptr) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
  }
  removeCallbacks();
}

void MouseHook::addCallback(const EventCallback &callback) {
  m_callbacks.push_back(callback);
}

void MouseHook::removeCallbacks() { m_callbacks.clear(); }

LRESULT CALLBACK MouseHook::HookCallback(int nCode, WPARAM wParam,
                                         LPARAM lParam) {
  if (nCode >= HC_ACTION) {
    auto &instance = GetInstance();
    for (const auto &callback : instance.m_callbacks) {
      callback(nCode, wParam, lParam);
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace VirtualDesktop