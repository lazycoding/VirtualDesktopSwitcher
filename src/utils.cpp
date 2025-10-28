#include "utils.h"
#include <ShellScalingApi.h>
#include <cstdarg>
#include <iostream>

namespace VirtualDesktop {
void Trace(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[1024] = {0};
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    std::cout << buffer << std::endl;
}
void GetScaleForMonitor(HWND hwnd, float& scaleX, float& scaleY) {
    // Get current DPI for scaling
    UINT dpiX = 96, dpiY = 96;
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (hMonitor) {
        GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    }
    scaleX = static_cast<float>(dpiX) / 96.0f;
    scaleY = static_cast<float>(dpiY) / 96.0f;
}
}  // namespace VirtualDesktop