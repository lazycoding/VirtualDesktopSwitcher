#pragma once
#include <Windows.h>

#include <string>


namespace VirtualDesktop {
class Settings;  // Forward declaration
}

namespace VirtualDesktop {

/**
 * @brief Provides UI for application settings configuration
 */
class SettingsView {
 public:
  /**
   * @brief Creates and shows the settings window
   * @param hInstance Application instance handle
   * @param parentWindow Parent window handle
   * @return true if window created successfully
   */
  bool show(HINSTANCE hInstance, HWND parentWindow);

  /**
   * @brief Hides the settings window
   */
  void hide();

  /**
   * @brief Gets current gesture sensitivity from UI
   * @return Sensitivity value (1-10)
   */
  int getGestureSensitivity() const;

  /**
   * @brief Loads settings from Settings object
   * @param settings Settings object to load from
   */
  void loadSettings(const Settings &settings);

  /**
   * @brief Saves settings to Settings object
   * @param settings Settings object to save to
   */
  void saveSettings(Settings &settings) const;

  /**
   * @brief Gets current overlay color from UI
   * @return Color in hex format (#RRGGBBAA)
   */
  std::wstring getOverlayColor() const;

  SettingsView() : m_currentColor(RGB(255, 255, 255)) {}
  ~SettingsView() { hide(); }

 private:
  // Disable copy and move
  SettingsView(const SettingsView &) = delete;
  SettingsView &operator=(const SettingsView &) = delete;
  SettingsView(SettingsView &&) = delete;
  SettingsView &operator=(SettingsView &&) = delete;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd = nullptr;
  COLORREF m_currentColor = RGB(255, 255, 255);  // Default white color
  HWND m_colorPreview = nullptr;
  DWORD m_customColors[16] = {0};  // Custom color palette
};

}  // namespace VirtualDesktop