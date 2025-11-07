#pragma once
#include <Windows.h>

namespace VirtualDesktop {

/**
 * @brief Manages virtual desktop operations on Windows
 */
class DesktopManager {
public:
    /**
     * @brief Switches to the next desktop
     * @param direction true for right, false for left
     * @return true if successful
     */
    bool switchDesktop(bool direction) const;

    DesktopManager() = default;
    ~DesktopManager() = default;

private:
    // Disable copy and move
    DesktopManager(const DesktopManager&) = delete;
    DesktopManager& operator=(const DesktopManager&) = delete;
    DesktopManager(DesktopManager&&) = delete;
    DesktopManager& operator=(DesktopManager&&) = delete;
};

}  // namespace VirtualDesktop