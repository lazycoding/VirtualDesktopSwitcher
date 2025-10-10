#include "MouseHook.h"
#include <stdexcept>

// 静态成员初始化
HHOOK MouseHook::hookHandle = nullptr;
MouseHook::EventCallback MouseHook::callback = nullptr;
POINT MouseHook::lastPosition = {0, 0};
bool MouseHook::isTracking = false;
int MouseHook::sensitivity = 50;
int MouseHook::threshold = 5;

bool MouseHook::Initialize(EventCallback cb) {
    if (hookHandle != nullptr) {
        throw std::runtime_error("Mouse hook already initialized");
    }
    
    callback = cb;
    hookHandle = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
    return hookHandle != nullptr;
}

void MouseHook::Shutdown() {
    if (hookHandle) {
        UnhookWindowsHookEx(hookHandle);
        hookHandle = nullptr;
    }
    callback = nullptr;
}

LRESULT CALLBACK MouseHook::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && callback) {
        PMSLLHOOKSTRUCT pMouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        MouseEvent event;
        event.position = pMouse->pt;
        event.timestamp = pMouse->time;
        
        switch (wParam) {
            case WM_XBUTTONDOWN:
                event.button = (HIWORD(pMouse->mouseData) == XBUTTON1) ? 
                    MouseEvent::Button::XButton1 : MouseEvent::Button::XButton2;
                event.state = MouseEvent::State::Pressed;
                lastPosition = pMouse->pt;
                isTracking = true;
                break;
                
            case WM_XBUTTONUP:
                event.button = (HIWORD(pMouse->mouseData) == XBUTTON1) ? 
                    MouseEvent::Button::XButton1 : MouseEvent::Button::XButton2;
                event.state = MouseEvent::State::Released;
                isTracking = false;
                break;
                
            case WM_MOUSEMOVE:
                if (isTracking) {
                    event.button = MouseEvent::Button::None;
                    event.state = MouseEvent::State::Move;
                    
                    // 滑动方向检测
                    int deltaX = pMouse->pt.x - lastPosition.x;
                    if (abs(deltaX) > sensitivity) {
                        // 触发滑动事件处理
                        callback(event);
                        lastPosition = pMouse->pt;
                    }
                }
                break;
        }
        
        if (callback && !callback(event)) {
            return 1; // 阻止事件继续传递
        }
    }
    return CallNextHookEx(hookHandle, nCode, wParam, lParam);
}

void MouseHook::SetSensitivity(int value) {
    sensitivity = max(10, min(200, value));
}

void MouseHook::SetThreshold(int value) {
    threshold = max(1, min(10, value));
}