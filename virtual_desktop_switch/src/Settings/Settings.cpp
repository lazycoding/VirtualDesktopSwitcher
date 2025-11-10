#include "Settings.h"
#include <Windows.h>
#include <fstream>
#include <string>

namespace VirtualDesktop {

namespace {
constexpr const char* DEFAULT_CONFIG = R"(
{
  "basic": {
    "auto_start": false,
    "tray_icon": true
  },
  "gesture": {
    "trigger_button": "X1",
    "sensitivity": 5,
    "line_width": 5,
    "color": "#6495EDAA"
  },
  "rendering": {
    "mode": "GDI+",
    "transparency": 80
  },
  "behavior": {
    "desktop_cycle": true,
    "desktop_preview": true,
    "switch_animation": true
  }
}
)";
}  // namespace

// mousebutton to string
std::string mouseButtonToString(MouseButton button) {
    switch (button) {
        case MouseButton::None:
            return "None";
        case MouseButton::Left:
            return "Left";
        case MouseButton::Right:
            return "Right";
        case MouseButton::X1:
            return "X1";
        case MouseButton::X2:
            return "X2";
        default:
            return "None";
    }
}

// string to mousebutton
MouseButton stringToMouseButton(const std::string& button) {
    // 忽略字母大小写
    std::string buttonLower = button;
    std::transform(buttonLower.begin(), buttonLower.end(), buttonLower.begin(), ::tolower);
    if (buttonLower == "x1") {
        return MouseButton::X1;
    } else if (buttonLower == "x2") {
        return MouseButton::X2;
    } else if (buttonLower == "left") {
        return MouseButton::Left;
    } else if (buttonLower == "right") {
        return MouseButton::Right;
    }
    return MouseButton::None;
}

bool Settings::load(const std::wstring& filePath) {
    try {
        std::ifstream file(filePath.c_str());
        if (!file.is_open()) {
            m_config = nlohmann::json::parse(DEFAULT_CONFIG);
            return false;
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

// Basic settings
bool Settings::isAutoStartEnabled() const {
    return m_config.value("basic/auto_start", false);
}

void Settings::setAutoStartEnabled(bool enabled) {
    m_config["basic/auto_start"] = enabled;
}

bool Settings::isTrayIconEnabled() const {
    return m_config.value("basic/tray_icon", true);
}

void Settings::setTrayIconEnabled(bool enabled) {
    m_config["basic/tray_icon"] = enabled;
}

// Gesture settings
MouseButton Settings::getTriggerButton() const {
    std::string button = m_config.value("gesture/trigger_button", "X1");
    return stringToMouseButton(button);
}

void Settings::setTriggerButton(MouseButton button) {
    m_config["gesture/trigger_button"] = mouseButtonToString(button);
}

int Settings::getGestureSensitivity() const {
    return m_config.value("gesture/sensitivity", 5);
}

void Settings::setGestureSensitivity(int value) {
    m_config["gesture/sensitivity"] = std::clamp(value, 1, 10);
}

std::string Settings::getOverlayColor() const {
    std::string color = m_config.value("gesture/color", "#6495EDAA");
    return color;
}

void Settings::setOverlayColor(const std::string& color) {
    if (color.size() == 9 && color[0] == L'#') {
        m_config["gesture/color"] = color;
    }
}

int Settings::getGestureLineWidth() const {
    return m_config.value("gesture/line_width", 5);
}

void Settings::setGestureLineWidth(int width) {
    m_config["gesture/line_width"] = std::clamp(width, 1, 10);
}

// Rendering settings
RenderMode Settings::getRenderingMode() const {
    auto mode = RenderMode(m_config.value("rendering/mode", RenderMode::Gdiplus));
    return mode;
}

void Settings::setRenderingMode(RenderMode mode) {
    if (mode == RenderMode::Gdiplus || mode == RenderMode::Direct2D) {
        m_config["rendering/mode"] = mode;
    }
}

int Settings::getTransparency() const {
    return m_config.value("rendering/transparency", 80);
}

void Settings::setTransparency(int value) {
    m_config["rendering/transparency"] = std::clamp(value, 0, 100);
}

// Behavior settings
bool Settings::isDesktopCycleEnabled() const {
    return m_config.value("behavior/desktop_cycle", true);
}

void Settings::setDesktopCycleEnabled(bool enabled) {
    m_config["behavior/desktop_cycle"] = enabled;
}

bool Settings::isDesktopPreviewEnabled() const {
    return m_config.value("behavior/desktop_preview", true);
}

void Settings::setDesktopPreviewEnabled(bool enabled) {
    m_config["behavior/desktop_preview"] = enabled;
}

bool Settings::isSwitchAnimationEnabled() const {
    return m_config.value("behavior/switch_animation", true);
}

void Settings::setSwitchAnimationEnabled(bool enabled) {
    m_config["behavior/switch_animation"] = enabled;
}

}  // namespace VirtualDesktop