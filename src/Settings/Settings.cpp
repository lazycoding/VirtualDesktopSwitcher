#include "Settings/Settings.h"
#include "utils.h"
#include <Windows.h>
#include <fstream>
#include <string>

namespace VirtualDesktop {

namespace {
constexpr const char* DEFAULT_CONFIG = R"(
    {
        "gesture_line_width": 5,
        "overlay_color": "#6495EDAA",
        "rendering_mode": "GDI+"
    }
)";
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

std::wstring Settings::getRenderingMode() const {
    std::string mode = m_config.value("rendering_mode", "Direct2D");
    return utf8_decode(mode);
}

void Settings::setRenderingMode(const std::wstring& mode) {
    if (mode == L"GDI+" || mode == L"Direct2D") {
        m_config["rendering_mode"] = utf8_encode(mode);
    }
}

}  // namespace VirtualDesktop