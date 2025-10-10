#pragma once
#include <functional>
#include <Windows.h>

struct MouseEvent {
    enum class Button { None, XButton1, XButton2 };
    enum class State { Pressed, Released, Move };
    
    Button button;
    State state;
    POINT position;
    DWORD timestamp;
};

class MouseHook {
public:
    using EventCallback = std::function<bool(const MouseEvent&)>;
    
    bool Initialize(EventCallback callback);
    void Shutdown();
    
    // 滑动检测参数配置
    void SetSensitivity(int sensitivity);
    void SetThreshold(int threshold);

private:
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    HHOOK hookHandle = nullptr;
    EventCallback callback;
    POINT lastPosition = {0, 0};
    bool isTracking = false;
    
    // 配置参数
    int sensitivity = 50; // 像素阈值
    int threshold = 5;    // 灵敏度系数
};