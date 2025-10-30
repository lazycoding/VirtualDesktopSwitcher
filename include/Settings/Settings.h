#pragma once
#include <string>
#include "nlohmann/json.hpp"

namespace VirtualDesktop {

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

    /**
     * @brief Gets overlay color
     * @return Color in hex format (#RRGGBBAA)
     */
    std::wstring getOverlayColor() const;

    /**
     * @brief Sets overlay color
     * @param color Color in hex format (#RRGGBBAA)
     */
    void setOverlayColor(const std::wstring& color);

    /**
     * @brief Gets gesture line width
     * @return Width of gesture lines in pixels
     */
    int getGestureLineWidth() const;

    /**
     * @brief Sets gesture line width
     * @param value Width of gesture lines in pixels
     */
    void setGestureLineWidth(int value);

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