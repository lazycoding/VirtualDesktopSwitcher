#pragma once
#include <Windows.h>

class MouseHook {
public:
  static MouseHook &GetInstance();

  bool Initialize();
  void Shutdown();

private:
  MouseHook() = default;
  ~MouseHook() = default;

  static LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam);

  HHOOK m_hook = nullptr;
};