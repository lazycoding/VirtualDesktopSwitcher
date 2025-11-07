#pragma once
#include "IRenderer.h"
#include "Settings/Settings.h"
#include <Windows.h>
#include <vector>
#include <memory>

namespace VirtualDesktop {
/**
 * @brief Renders overlay UI for gesture visualization
 */
class OverlayUI {
public:
    OverlayUI();
    ~OverlayUI();

    /**
     * @brief Initializes the renderer based on settings
     * @return true if initialization succeeded
     */
    bool initialize(HINSTANCE hInstance);

    /**
     * @brief Shows the overlay window
     */
    void show();

    /**
     * @brief Hides the overlay window and clears the trajectory points
     */
    void hide();

    /**
     * @brief Updates the mouse position and renders the gesture trajectory
     * @param x The x coordinate of the mouse position
     * @param y The y coordinate of the mouse position
     */
    void updatePosition(int x, int y);

    /**
     * @brief Renders the gesture path
     * @param points Vector of mouse positions
     */
    void render(const std::vector<POINT>& points);

    /**
     * @brief Clears the overlay
     */
    void clear();

    /**
     * @brief Sets the settings for the overlay
     * @param settings The settings object
     */
    void setSettings(const Settings& settings);

private:
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void switchRenderer();

private:
    std::unique_ptr<IRenderer> m_renderer;  // Active renderer created by factory
    std::vector<POINT> m_trajectoryPoints;
    std::vector<POINT> m_smoothedPoints;  // For storing smoothed trajectory
    HWND m_hWnd = nullptr;
    const Settings* m_settings;  // Pointer to settings instead of copy

    // Smooth the trajectory points for less jittery display
    void smoothTrajectory();
};

}  // namespace VirtualDesktop