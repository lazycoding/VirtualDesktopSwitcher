#pragma once
#include <string>
#include "nlohmann/json.hpp"

namespace VirtualDesktop {

// 定义鼠标切换键枚举类型
enum class MouseButton { None, Left, Right, X1, X2 };

enum class RenderMode { Direct2D, Gdiplus };

// mousebutton to string
std::string mouseButtonToString(MouseButton button);

// string to mousebutton
MouseButton stringToMouseButton(const std::string& button);

/**
 * @brief Manages application configuration
 */
class Settings {
public:
    /**
     * @brief Loads settings from config file
     * @param filePath Path to config file
     * @return true if loaded successfully
     */
    bool load(const std::wstring& filePath);

    /**
     * @brief Saves settings to config file
     * @param filePath Path to config file
     * @return true if saved successfully
     */
    bool save(const std::wstring& filePath) const;

    // Basic settings
    bool isAutoStartEnabled() const;
    void setAutoStartEnabled(bool enabled);
    bool isTrayIconEnabled() const;
    void setTrayIconEnabled(bool enabled);

    // Gesture settings
    MouseButton getTriggerButton() const;
    void setTriggerButton(MouseButton button);
    int getGestureSensitivity() const;
    void setGestureSensitivity(int value);
    std::string getOverlayColor() const;
    void setOverlayColor(const std::string& color);
    int getGestureLineWidth() const;
    void setGestureLineWidth(int value);

    // Rendering settings
    RenderMode getRenderingMode() const;
    void setRenderingMode(RenderMode mode);
    int getTransparency() const;
    void setTransparency(int value);

    // Behavior settings
    bool isDesktopCycleEnabled() const;
    void setDesktopCycleEnabled(bool enabled);
    bool isDesktopPreviewEnabled() const;
    void setDesktopPreviewEnabled(bool enabled);
    bool isSwitchAnimationEnabled() const;
    void setSwitchAnimationEnabled(bool enabled);

    Settings() = default;
    ~Settings() = default;

private:
    // Disable copy and move
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings&&) = delete;

    nlohmann::json m_config;
};

}  // namespace VirtualDesktop