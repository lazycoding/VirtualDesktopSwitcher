#pragma once
#include <Windows.h>
#include <string>
namespace VirtualDesktop {

void trace(const char* format, ...);

void getScaleForMonitor(HWND hwnd, float& scaleX, float& scaleY);

std::string utf8_encode(const std::wstring& wide_str);

std::wstring utf8_decode(const std::string& utf8_str);

}  // namespace VirtualDesktop
