#pragma once
#include <Windows.h>
#include <string>
#include <vector>


namespace VirtualDesktop {

/**
 * @brief Manages virtual desktop operations on Windows
 */
class DesktopManager {
public:
  /**
   * @brief Gets the singleton instance
   */
  static DesktopManager &GetInstance();

  /**
   * @brief Enumerates all virtual desktops
   * @return Vector of desktop names
   */
  std::vector<std::wstring> enumerateDesktops() const;

  /**
   * @brief Switches to the next desktop
   * @param direction true for right, false for left
   * @return true if successful
   */
  bool switchDesktop(bool direction) const;

  /**
   * @brief Gets current desktop name
   * @return Current desktop name
   */
  std::wstring getCurrentDesktopName() const;

private:
  DesktopManager() = default;
  ~DesktopManager() = default;

  // Disable copy and move
  DesktopManager(const DesktopManager &) = delete;
  DesktopManager &operator=(const DesktopManager &) = delete;
  DesktopManager(DesktopManager &&) = delete;
  DesktopManager &operator=(DesktopManager &&) = delete;
};

} // namespace VirtualDesktop