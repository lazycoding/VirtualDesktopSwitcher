#include "OverlayUI/GdiRenderer.h"
#include <algorithm>

namespace VirtualDesktop {

GdiRenderer::GdiRenderer() :
        m_hwnd(nullptr),
        m_windowDC(nullptr),
        m_memoryDC(nullptr),
        m_bitmap(nullptr),
        m_oldBitmap(nullptr),
        m_backgroundBrush(nullptr),
        m_pen(nullptr),
        m_trailColor(RGB(100, 149, 237))  // Default: Cornflower Blue
        ,
        m_lineWidth(5.0f),
        m_width(0),
        m_height(0) {
}

GdiRenderer::~GdiRenderer() {
    // Clean up GDI resources
    if (m_pen) {
        DeleteObject(m_pen);
        m_pen = nullptr;
    }

    if (m_backgroundBrush) {
        DeleteObject(m_backgroundBrush);
        m_backgroundBrush = nullptr;
    }

    if (m_memoryDC && m_oldBitmap) {
        SelectObject(m_memoryDC, m_oldBitmap);
        m_oldBitmap = nullptr;
    }

    if (m_bitmap) {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }

    if (m_memoryDC) {
        DeleteDC(m_memoryDC);
        m_memoryDC = nullptr;
    }

    if (m_windowDC) {
        ReleaseDC(m_hwnd, m_windowDC);
        m_windowDC = nullptr;
    }
}
namespace {
constexpr COLORREF DEFAULT_TRAIL_COLOR = RGB(100, 149, 237);  // Cornflower Blue

bool parseHexComponent(const std::string& hex, size_t offset, BYTE& result) {
    try {
        result = static_cast<BYTE>(std::stoul(hex.substr(offset, 2), nullptr, 16));
        return true;
    } catch (...) {
        return false;
    }
}
}  // namespace

COLORREF GdiRenderer::hexToCOLORREF(const std::string& hex) {
    if (hex.length() != 9 || hex[0] != '#') {
        return DEFAULT_TRAIL_COLOR;
    }

    BYTE r, g, b;
    if (parseHexComponent(hex, 1, r) && parseHexComponent(hex, 3, g) && parseHexComponent(hex, 5, b)) {
        return RGB(r, g, b);
    }
    return DEFAULT_TRAIL_COLOR;
}

void GdiRenderer::setTrailStyle(const std::string& colorHex, float lineWidth) {
    m_trailColor = hexToCOLORREF(colorHex);
    m_lineWidth = std::clamp(lineWidth, 1.0f, 10.0f);

    if (m_pen) {
        DeleteObject(m_pen);
    }

    m_pen = CreatePen(PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, static_cast<int>(m_lineWidth), m_trailColor);
}

void GdiRenderer::computeVirtualScreenRect() {
    // Use virtual screen metrics for multi-monitor support
    m_rcVirtual.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_rcVirtual.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_rcVirtual.right = m_rcVirtual.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_rcVirtual.bottom = m_rcVirtual.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_width = std::max((LONG)1, m_rcVirtual.right - m_rcVirtual.left);
    m_height = std::max((LONG)1, m_rcVirtual.bottom - m_rcVirtual.top);
}

bool GdiRenderer::initialize(HWND hwndParent) {
    m_hwnd = hwndParent;

    // Compute virtual screen bounds
    computeVirtualScreenRect();

    // Get the window DC
    m_windowDC = GetDC(m_hwnd);

    // Create memory DC and compatible bitmap
    m_memoryDC = CreateCompatibleDC(m_windowDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_width;
    bmi.bmiHeader.biHeight = -m_height;  // Negative for top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    m_bitmap = CreateDIBSection(m_windowDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    if (!m_bitmap) {
        ReleaseDC(m_hwnd, m_windowDC);
        return false;
    }

    m_oldBitmap = (HBITMAP)SelectObject(m_memoryDC, m_bitmap);

    // Create background brush (transparent)
    m_backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));  // Black background (will be made transparent)

    // Initialize pen
    m_pen = CreatePen(PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, static_cast<int>(m_lineWidth), m_trailColor);

    return true;
}

void GdiRenderer::resizeForMonitors() {
    // Clean up old resources
    if (m_pen) {
        DeleteObject(m_pen);
        m_pen = nullptr;
    }

    if (m_backgroundBrush) {
        DeleteObject(m_backgroundBrush);
        m_backgroundBrush = nullptr;
    }

    if (m_memoryDC && m_oldBitmap) {
        SelectObject(m_memoryDC, m_oldBitmap);
        m_oldBitmap = nullptr;
    }

    if (m_bitmap) {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }

    if (m_memoryDC) {
        DeleteDC(m_memoryDC);
        m_memoryDC = nullptr;
    }

    // Reinitialize with new dimensions
    computeVirtualScreenRect();

    m_memoryDC = CreateCompatibleDC(m_windowDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_width;
    bmi.bmiHeader.biHeight = -m_height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    m_bitmap = CreateDIBSection(m_windowDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    if (m_bitmap) {
        m_oldBitmap = (HBITMAP)SelectObject(m_memoryDC, m_bitmap);
        m_backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
        m_pen = CreatePen(PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, static_cast<int>(m_lineWidth), m_trailColor);
    }
}

void GdiRenderer::drawSmoothTrail(const std::vector<POINT>& points) {
    if (points.size() < 2)
        return;

    // Convert points to memory DC coordinates
    std::vector<POINT> convertedPoints;
    convertedPoints.reserve(points.size());

    for (const auto& pt : points) {
        POINT convertedPt = {pt.x - m_rcVirtual.left, pt.y - m_rcVirtual.top};
        convertedPoints.push_back(convertedPt);
    }

    // Draw the entire polyline at once for smoother appearance
    SelectObject(m_memoryDC, m_pen);
    Polyline(m_memoryDC, convertedPoints.data(), static_cast<int>(convertedPoints.size()));
}

void GdiRenderer::render(const std::vector<POINT>& points) {
    if (points.size() < 2 || !m_memoryDC || !m_pen) {
        return;
    }

    // Enable anti-aliasing for smoother lines
    SetGraphicsMode(m_memoryDC, GM_ADVANCED);
    SetBkMode(m_memoryDC, TRANSPARENT);

    // Set high-quality drawing
    SetROP2(m_memoryDC, R2_COPYPEN);

    // Clear the background to transparent (black)
    RECT rc = {0, 0, m_width, m_height};
    FillRect(m_memoryDC, &rc, m_backgroundBrush);

    // Draw the trail with enhanced smoothness
    drawSmoothTrail(points);

    // Update the layered window with the rendered bitmap
    HDC screenDC = GetDC(NULL);
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    POINT srcPoint = {0, 0};
    SIZE size = {m_width, m_height};
    POINT destPoint = {m_rcVirtual.left, m_rcVirtual.top};

    // Use UpdateLayeredWindow to display the rendered bitmap
    UpdateLayeredWindow(m_hwnd, screenDC, &destPoint, &size, m_memoryDC, &srcPoint, 0, &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

void GdiRenderer::clear() {
    if (!m_memoryDC) {
        return;
    }

    // Clear to transparent (black)
    RECT rc = {0, 0, m_width, m_height};
    FillRect(m_memoryDC, &rc, m_backgroundBrush);

    // Update the display to clear the trail
    HDC screenDC = GetDC(NULL);
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    POINT srcPoint = {0, 0};
    SIZE size = {m_width, m_height};
    POINT destPoint = {m_rcVirtual.left, m_rcVirtual.top};

    UpdateLayeredWindow(m_hwnd, screenDC, &destPoint, &size, m_memoryDC, &srcPoint, 0, &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

}  // namespace VirtualDesktop