#pragma once
#include <string>
#include "nlohmann/json.hpp"

namespace VirtualDesktop {

/**
 * @brief Manages application configuration
 */
class Settings {
public:
    // Centralized rendering mode constants (UTF-8 and wide variants)
    static inline const char* RENDERING_MODE_DIRECT2D = "Direct2D";
    static inline const char* RENDERING_MODE_GDIPLUS = "GDI+";
    static inline const std::wstring RENDERING_MODE_DIRECT2D_W = L"Direct2D";
    static inline const std::wstring RENDERING_MODE_GDIPLUS_W = L"GDI+";

    // Mouse button constants
    static inline const char* MOUSE_BUTTON_SIDE1 = "Side1";
    static inline const char* MOUSE_BUTTON_SIDE2 = "Side2";
    static inline const std::wstring MOUSE_BUTTON_SIDE1_W = L"Side1";
    static inline const std::wstring MOUSE_BUTTON_SIDE2_W = L"Side2";

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
    std::wstring getTriggerButton() const;
    void setTriggerButton(const std::wstring& button);
    int getGestureSensitivity() const;
    void setGestureSensitivity(int value);
    std::wstring getOverlayColor() const;
    void setOverlayColor(const std::wstring& color);
    int getGestureLineWidth() const;
    void setGestureLineWidth(int value);

    // Rendering settings
    std::wstring getRenderingMode() const;
    void setRenderingMode(const std::wstring& mode);
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