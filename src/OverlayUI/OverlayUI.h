#pragma once
#include "MouseTrailRenderer.h"
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
         * @brief Initializes Direct2D resources
         * @return true if initialization succeeded
         */
        bool initialize(HINSTANCE hInst);

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
    private:
        static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam,
            LPARAM lParam);

    private:
        MouseTrailRenderer m_mouseTrailRenderer;
        std::vector<POINT> m_trajectoryPoints;
        HWND m_hWnd;
    };

} // namespace VirtualDesktop