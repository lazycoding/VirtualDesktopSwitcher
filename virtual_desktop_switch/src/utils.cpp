#include "utils.h"
#include <ShellScalingApi.h>
#include <cstdarg>
#include <iostream>

namespace VirtualDesktop {
void trace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024] = {0};
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    std::cout << buffer << std::endl;
}
void getScaleForMonitor(HWND hwnd, float& scaleX, float& scaleY) {
    // Get current DPI for scaling
    UINT dpiX = 96, dpiY = 96;
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (hMonitor) {
        GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    }
    scaleX = static_cast<float>(dpiX) / 96.0f;
    scaleY = static_cast<float>(dpiY) / 96.0f;
}
// unicode to utf8
std::string utf8_encode(const std::wstring& wide_str) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_str[0], (int)wide_str.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wide_str[0], (int)wide_str.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring utf8_decode(const std::string& utf8_str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), NULL, 0);
    std::wstring strTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), &strTo[0], size_needed);
    return strTo;
}

bool parseHexComponent(const std::string& hex, size_t offset, float& result) {
    try {
        result = static_cast<float>(std::stoul(hex.substr(offset, 2), nullptr, 16)) / 255.0f;
        return true;
    } catch (...) {
        return false;
    }
}
}  // namespace VirtualDesktop