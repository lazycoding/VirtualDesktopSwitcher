#include "DesktopManager.h"
#include <string>
#include <vector>
#include <windows.h>


namespace VirtualDesktop {

namespace {
constexpr int KEYEVENTF_KEYUP = 0x0002;
constexpr int VK_CONTROL = 0x11;
constexpr int VK_WIN = 0x5B;
constexpr int VK_LEFT = 0x25;
constexpr int VK_RIGHT = 0x27;
} // namespace

DesktopManager &DesktopManager::GetInstance() {
  static DesktopManager instance;
  return instance;
}

std::vector<std::wstring> DesktopManager::enumerateDesktops() const {
  // TODO: Implement actual desktop enumeration
  return {L"Desktop 1", L"Desktop 2", L"Desktop 3"};
}

bool DesktopManager::switchDesktop(bool direction) const {
  INPUT inputs[4] = {};
  ZeroMemory(inputs, sizeof(inputs));

  // Press Ctrl+Win
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_CONTROL;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = VK_WIN;

  // Press arrow key
  inputs[2].type = INPUT_KEYBOARD;
  inputs[2].ki.wVk = direction ? VK_RIGHT : VK_LEFT;

  // Release all keys
  inputs[3] = inputs[2];
  inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[4] = inputs[1];
  inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[5] = inputs[0];
  inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

  return SendInput(6, inputs, sizeof(INPUT)) == 6;
}

std::wstring DesktopManager::getCurrentDesktopName() const {
  // TODO: Implement actual current desktop detection
  return L"Desktop 1";
}

} // namespace VirtualDesktop