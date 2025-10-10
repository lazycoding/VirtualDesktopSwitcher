#pragma once
#include <Windows.h>
#include <functional>

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
  using EventCallback = std::function<bool(const MouseEvent &)>;

  bool Initialize(EventCallback callback);
  void Shutdown();

  // 滑动检测参数配置
  void SetSensitivity(int sensitivity);
  void SetThreshold(int threshold);

private:
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam,
                                            LPARAM lParam);

  static HHOOK hookHandle;
  static EventCallback callback;
  static POINT lastPosition;
  static bool isTracking;

  // 配置参数
  static int sensitivity; // 像素阈值
  static int threshold;   // 灵敏度系数
};