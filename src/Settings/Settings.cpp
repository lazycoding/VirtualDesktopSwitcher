#include "Settings.h"
#include <Windows.h>
#include <fstream>
#include <string>

namespace VirtualDesktop {

namespace {
constexpr const char* DEFAULT_CONFIG = R"(
    {
        "gesture_line_width": 5,
        "overlay_color": "#6495EDAA"
    }
)";

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
}  // namespace

bool Settings::load(const std::wstring& filePath) {
    try {
        std::ifstream file(filePath.c_str());
        if (!file.is_open()) {
            m_config = nlohmann::json::parse(DEFAULT_CONFIG);
        } else {
            m_config = nlohmann::json::parse(file);
        }
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

bool Settings::save(const std::wstring& filePath) const {
    try {
        std::ofstream file(filePath.c_str());
        if (!file.is_open()) {
            return false;
        }
        file << m_config.dump(4);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::wstring Settings::getOverlayColor() const {
    std::string color = m_config.value("overlay_color", "#6495EDAA");
    return utf8_decode(color);
}

void Settings::setOverlayColor(const std::wstring& color) {
    if (color.size() == 9 && color[0] == L'#') {
        m_config["overlay_color"] = utf8_encode(color);
    }
}

int Settings::getGestureLineWidth() const {
    return m_config.value("gesture_line_width", 3);
}

void Settings::setGestureLineWidth(int width) {
    m_config["gesture_line_width"] = std::clamp(width, 1, 10);
}

}  // namespace VirtualDesktop