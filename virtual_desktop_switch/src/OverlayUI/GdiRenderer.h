#pragma once
#include "OverlayUI/IRenderer.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace VirtualDesktop {

/**
 * @brief Renders mouse trail using Windows GDI for gesture visualization
 */
class GdiRenderer : public IRenderer {
public:
    GdiRenderer();
    ~GdiRenderer();

    /**
     * @brief Sets the trail style (color and line width)
     * @param colorHex Color in hex format (#RRGGBBAA)
     * @param lineWidth Width of the trail line
     */
    void setTrailStyle(const std::string& colorHex, float lineWidth) override;

    /**
     * @brief Initializes the renderer with parent window
     * @param hwndParent Parent window handle
     * @return true if initialization succeeded
     */
    bool initialize(HWND hwndParent) override;

    /**
     * @brief Resizes the renderer for multi-monitor setups
     */
    void resizeForMonitors() override;

    /**
     * @brief Renders the mouse trail
     * @param points Vector of mouse positions
     */
    void render(const std::vector<POINT>& points) override;

    /**
     * @brief Clears the rendered trail
     */
    void clear() override;

private:
    HWND m_hwnd;
    HDC m_windowDC;
    HDC m_memoryDC;
    HBITMAP m_bitmap;
    HBITMAP m_oldBitmap;
    HBRUSH m_backgroundBrush;
    HPEN m_pen;
    COLORREF m_trailColor;
    float m_lineWidth;
    RECT m_rcVirtual;
    LONG m_width;
    LONG m_height;

    COLORREF hexToCOLORREF(const std::string& hex);
    void computeVirtualScreenRect();

    // Draw smooth lines using Polyline instead of multiple LineTo calls
    void drawSmoothTrail(const std::vector<POINT>& points);

    // Disable copy and move
    GdiRenderer(const GdiRenderer&) = delete;
    GdiRenderer& operator=(const GdiRenderer&) = delete;
    GdiRenderer(GdiRenderer&&) = delete;
    GdiRenderer& operator=(GdiRenderer&&) = delete;
};

}  // namespace VirtualDesktop