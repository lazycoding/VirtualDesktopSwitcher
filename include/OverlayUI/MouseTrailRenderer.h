#pragma once
#include <Windows.h>
#include <d2d1_1.h>
#include <string>
#include <vector>
#include <array>

namespace VirtualDesktop {

/**
 * @brief Renders mouse trail using Direct2D for gesture visualization
 */
class MouseTrailRenderer {
public:
    MouseTrailRenderer();
    ~MouseTrailRenderer();

    /**
     * @brief Sets the trail style (color and line width)
     * @param colorHex Color in hex format (#RRGGBBAA)
     * @param lineWidth Width of the trail line
     */
    void setTrailStyle(const std::wstring& colorHex, float lineWidth);

    /**
     * @brief Initializes the renderer with parent window
     * @param hwndParent Parent window handle
     * @return true if initialization succeeded
     */
    bool initialize(HWND hwndParent);

    /**
     * @brief Resizes the renderer for multi-monitor setups
     */
    void resizeForMonitors();

    /**
     * @brief Renders the mouse trail
     * @param points Vector of mouse positions
     */
    void render(const std::vector<POINT>& points);

    /**
     * @brief Clears the rendered trail
     */
    void clear();

private:
    struct FPoint {
        float x;
        float y;
    };

    D2D1_COLOR_F hexToColorF(const std::wstring& hex);
    static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM data);
    void computeVirtualScreenRect();
    void createDIBAndRenderTarget();
    void destroyDIBAndRenderTarget();
    static FPoint quadraticBezier(const FPoint& p0, const FPoint& p1, const FPoint& p2, float t);
    static void precomputeBezierTable();
    static constexpr int BEZIER_TABLE_SIZE = 64;
    static inline std::array<float, BEZIER_TABLE_SIZE> s_bezierTable = {};

    ID2D1Factory* m_factory = nullptr;
    ID2D1DCRenderTarget* m_renderTarget = nullptr;
    ID2D1SolidColorBrush* m_brush = nullptr;
    ID2D1StrokeStyle* m_stroke = nullptr;
    HDC m_memDC = nullptr;
    HBITMAP m_hDIB = nullptr;
    void* m_pBits = nullptr;
    HWND m_hwnd = nullptr;
    RECT m_rcVirtual = {};
    LONG m_width = 0;
    LONG m_height = 0;
    D2D1_COLOR_F m_trailColor = D2D1::ColorF(D2D1::ColorF::SkyBlue, 0.9f);
    float m_lineWidth = 3.0f;
    static constexpr int STEPS = 10;

    // Disable copy and move
    MouseTrailRenderer(const MouseTrailRenderer&) = delete;
    MouseTrailRenderer& operator=(const MouseTrailRenderer&) = delete;
    MouseTrailRenderer(MouseTrailRenderer&&) = delete;
    MouseTrailRenderer& operator=(MouseTrailRenderer&&) = delete;
};

}  // namespace VirtualDesktop