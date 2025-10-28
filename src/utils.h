#pragma once
#include <Windows.h>
namespace VirtualDesktop {
void Trace(const char* fmt, ...);
void GetScaleForMonitor(HWND hwnd, float& scaleX, float& scaleY);
};  // namespace VirtualDesktop