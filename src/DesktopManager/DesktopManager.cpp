#include "DesktopManager/DesktopManager.h"
#include <windows.h>

namespace VirtualDesktop {

bool DesktopManager::switchDesktop(bool direction) const {
    INPUT inputs[6] = {};
    ZeroMemory(inputs, sizeof(inputs));

    // Press Ctrl+Win
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_LWIN;

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

}  // namespace VirtualDesktop