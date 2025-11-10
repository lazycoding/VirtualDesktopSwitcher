#pragma once
#include <Windows.h>
#include <string>
namespace VirtualDesktop {

void trace(const char* format, ...);

void getScaleForMonitor(HWND hwnd, float& scaleX, float& scaleY);

std::string utf8_encode(const std::wstring& wide_str);

std::wstring utf8_decode(const std::string& utf8_str);

/**
 * @brief Parses a hexadecimal color component from a string
 * @param hex Hexadecimal color string
 * @param offset Starting offset in the string
 * @param result Output float value (0.0f to 1.0f)
 * @return true if parsing succeeded, false otherwise
 */
bool parseHexComponent(const std::string& hex, size_t offset, float& result);

}  // namespace VirtualDesktop
