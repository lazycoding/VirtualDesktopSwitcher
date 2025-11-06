#pragma once
#include "MouseTrailRenderer.h"
#include "GdiRenderer.h"
#include "Settings/Settings.h"
#include <Windows.h>
#include <vector>

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
    MouseTrailRenderer m_direct2dRenderer;
    GdiRenderer m_gdiRenderer;
    bool m_useDirect2D;  // Flag to indicate which renderer to use
    std::vector<POINT> m_trajectoryPoints;
    std::vector<POINT> m_smoothedPoints; // For storing smoothed trajectory
    HWND m_hWnd;
    const Settings* m_settings;  // Pointer to settings instead of copy
    
    // Smooth the trajectory points for less jittery display
    void smoothTrajectory();
};

}  // namespace VirtualDesktop