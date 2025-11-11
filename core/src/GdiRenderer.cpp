#include "GdiRenderer.h"
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstring>

namespace VirtualDesktop {

GdiRenderer::GdiRenderer() :
        m_hwnd(nullptr),
        m_windowDC(nullptr),
        m_memoryDC(nullptr),
        m_bitmap(nullptr),
        m_oldBitmap(nullptr),
        m_backgroundBrush(nullptr),
        m_pen(nullptr),
        m_trailColor(RGB(100, 149, 237)),  // Default: Cornflower Blue
        m_alpha(0xAA),
        m_lineWidth(5.0f),
        m_width(0),
        m_height(0),
        m_pBits(nullptr) {
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

    if (m_windowDC && m_hwnd) {
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

// Helper: convert RGBA (0-255) to premultiplied BGRA dword
// inline uint32_t premultiplied_bgra(BYTE r, BYTE g, BYTE b, BYTE a) {
//     // premultiply color channels by alpha
//     float af = a / 255.0f;
//     BYTE pr = static_cast<BYTE>(std::round(r * af));
//     BYTE pg = static_cast<BYTE>(std::round(g * af));
//     BYTE pb = static_cast<BYTE>(std::round(b * af));
//     return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(pr) << 16) | (static_cast<uint32_t>(pg) << 8) |
//             (static_cast<uint32_t>(pb));
// }
}  // namespace

COLORREF GdiRenderer::hexToCOLORREF(const std::string& hex) {
    if (hex.length() != 9 || hex[0] != '#') {
        return DEFAULT_TRAIL_COLOR;
    }

    BYTE r, g, b, a;
    if (parseHexComponent(hex, 1, r) && parseHexComponent(hex, 3, g) && parseHexComponent(hex, 5, b) &&
        parseHexComponent(hex, 7, a)) {
        m_alpha = a;
        return RGB(r, g, b);
    }
    return DEFAULT_TRAIL_COLOR;
}

void GdiRenderer::setTrailStyle(const std::string& colorHex, float lineWidth) {
    m_trailColor = hexToCOLORREF(colorHex);
    m_lineWidth = std::clamp(lineWidth, 1.0f, 10.0f);

    if (m_pen) {
        DeleteObject(m_pen);
        m_pen = nullptr;
    }

    // Create a pen with the RGB color; alpha will be applied per-pixel in bitmap
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

    // Create memory DC
    m_memoryDC = CreateCompatibleDC(m_windowDC);

    // Create32-bit DIB section with alpha channel (BI_BITFIELDS)
    BITMAPV5HEADER bi = {};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = m_width;
    bi.bV5Height = -m_height;  // top-down
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    m_pBits = nullptr;
    m_bitmap = CreateDIBSection(m_windowDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &m_pBits, nullptr, 0);

    if (!m_bitmap || !m_pBits) {
        if (m_windowDC) {
            ReleaseDC(m_hwnd, m_windowDC);
            m_windowDC = nullptr;
        }
        return false;
    }

    m_oldBitmap = (HBITMAP)SelectObject(m_memoryDC, m_bitmap);

    // Create background brush (black) for clearing; actual alpha defined in pixel data
    m_backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));

    // Initialize pen
    if (m_pen == nullptr) {
        m_pen = CreatePen(PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, static_cast<int>(m_lineWidth), m_trailColor);
    }

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
        m_pBits = nullptr;
    }

    if (m_memoryDC) {
        DeleteDC(m_memoryDC);
        m_memoryDC = nullptr;
    }

    // Reinitialize with new dimensions
    computeVirtualScreenRect();

    m_memoryDC = CreateCompatibleDC(m_windowDC);

    BITMAPV5HEADER bi = {};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = m_width;
    bi.bV5Height = -m_height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    m_pBits = nullptr;
    m_bitmap = CreateDIBSection(m_windowDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &m_pBits, nullptr, 0);

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

// Postprocess the DIB bits: set per-pixel alpha where drawn pixels match trail color and make premultiplied
// Now only processes pixels inside the provided rectangle [left, top) x [right, bottom)
static void postprocess_bits_range(
        void* bits,
        int fullWidth,
        int fullHeight,
        int left,
        int top,
        int right,
        int bottom,
        COLORREF trailColor,
        BYTE alpha) {
    if (!bits)
        return;
    // Clip rectangle to bitmap bounds
    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > fullWidth)
        right = fullWidth;
    if (bottom > fullHeight)
        bottom = fullHeight;
    if (left >= right || top >= bottom)
        return;

    uint8_t* ptr = reinterpret_cast<uint8_t*>(bits);
    // bits are BGRA in memory (little endian: B,G,R,A)
    const BYTE tr = GetRValue(trailColor);
    const BYTE tg = GetGValue(trailColor);
    const BYTE tb = GetBValue(trailColor);

    for (int y = top; y < bottom; ++y) {
        // pointer to beginning of this row
        uint32_t* row = reinterpret_cast<uint32_t*>(ptr + y * fullWidth * 4);
        for (int x = left; x < right; ++x) {
            uint32_t pixel = row[x];
            BYTE b = static_cast<BYTE>(pixel & 0xFF);
            BYTE g = static_cast<BYTE>((pixel >> 8) & 0xFF);
            BYTE r = static_cast<BYTE>((pixel >> 16) & 0xFF);

            // If pixel is black (background) keep it fully transparent
            if (r == 0 && g == 0 && b == 0) {
                row[x] = 0x00000000;
                continue;
            }

            // If pixel matches the trail color (ignoring alpha), set requested alpha and premultiply
            if (r == tr && g == tg && b == tb) {
                // premultiply
                float af = alpha / 255.0f;
                BYTE pr = static_cast<BYTE>(std::round(r * af));
                BYTE pg = static_cast<BYTE>(std::round(g * af));
                BYTE pb = static_cast<BYTE>(std::round(b * af));
                row[x] = (static_cast<uint32_t>(alpha) << 24) | (static_cast<uint32_t>(pr) << 16) |
                        (static_cast<uint32_t>(pg) << 8) | (static_cast<uint32_t>(pb));
            } else {
                // For anti-aliased edges or unexpected colors, compute alpha based on intensity toward trail color
                int dr = static_cast<int>(r) - static_cast<int>(tr);
                int dg = static_cast<int>(g) - static_cast<int>(tg);
                int db = static_cast<int>(b) - static_cast<int>(tb);
                int dist2 = dr * dr + dg * dg + db * db;
                double dist = std::sqrt(static_cast<double>(dist2));
                double ratio = dist / 255.0;  // 0..~3
                double minVal = std::min(1.0, ratio);
                float sim = 1.0f - static_cast<float>(minVal);
                BYTE newA = static_cast<BYTE>(std::round(alpha * sim));
                float af = newA / 255.0f;
                BYTE pr = static_cast<BYTE>(std::round(r * af));
                BYTE pg = static_cast<BYTE>(std::round(g * af));
                BYTE pb = static_cast<BYTE>(std::round(b * af));
                row[x] = (static_cast<uint32_t>(newA) << 24) | (static_cast<uint32_t>(pr) << 16) |
                        (static_cast<uint32_t>(pg) << 8) | (static_cast<uint32_t>(pb));
            }
        }
    }
}

void GdiRenderer::render(const std::vector<POINT>& points) {
    if (points.size() < 2 || !m_memoryDC || !m_pen) {
        return;
    }

    // Set up drawing
    SetGraphicsMode(m_memoryDC, GM_ADVANCED);
    SetBkMode(m_memoryDC, TRANSPARENT);

    // Clear the background to black (transparent after postprocess)
    RECT rc = {0, 0, m_width, m_height};
    FillRect(m_memoryDC, &rc, m_backgroundBrush);

    // Compute converted points and bounding rectangle in memory coordinates
    std::vector<POINT> convertedPoints;
    convertedPoints.reserve(points.size());
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    for (const auto& pt : points) {
        POINT c = {pt.x - m_rcVirtual.left, pt.y - m_rcVirtual.top};
        convertedPoints.push_back(c);
        minX = std::min(minX, static_cast<int>(c.x));
        minY = std::min(minY, static_cast<int>(c.y));
        maxX = std::max(maxX, static_cast<int>(c.x));
        maxY = std::max(maxY, static_cast<int>(c.y));
    }

    // Add padding to include line width and antialiasing edges
    int pad = static_cast<int>(std::ceil(m_lineWidth)) + 2;
    int left = (minX == INT_MAX) ? 0 : (minX - pad);
    int top = (minY == INT_MAX) ? 0 : (minY - pad);
    int right = (maxX == INT_MIN) ? 0 : (maxX + pad + 1);
    int bottom = (maxY == INT_MIN) ? 0 : (maxY + pad + 1);

    // Clamp to bitmap
    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(m_width), right);
    bottom = std::min(static_cast<int>(m_height), bottom);

    // Draw the trail with enhanced smoothness
    drawSmoothTrail(points);

    // Postprocess only the bounding rectangle
    postprocess_bits_range(
            m_pBits,
            static_cast<int>(m_width),
            static_cast<int>(m_height),
            left,
            top,
            right,
            bottom,
            m_trailColor,
            m_alpha);

    // Update only the bounding rectangle area of the layered window
    HDC screenDC = GetDC(NULL);
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    POINT srcPoint = {left, top};
    SIZE size = {right - left, bottom - top};
    POINT destPoint = {m_rcVirtual.left + left, m_rcVirtual.top + top};

    // UpdateLayeredWindow will use the specified source rectangle from m_memoryDC
    UpdateLayeredWindow(m_hwnd, screenDC, &destPoint, &size, m_memoryDC, &srcPoint, 0, &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

void GdiRenderer::clear() {
    if (!m_memoryDC) {
        return;
    }

    // Clear to black
    RECT rc = {0, 0, m_width, m_height};
    FillRect(m_memoryDC, &rc, m_backgroundBrush);

    // Also clear pixel bits to zero (fully transparent)
    if (m_pBits) {
        memset(m_pBits, 0, static_cast<size_t>(m_width) * static_cast<size_t>(m_height) * 4);
    }

    // Update only the full window area when clearing (we clear whole bitmap)
    HDC screenDC = GetDC(NULL);
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    POINT srcPoint = {0, 0};
    SIZE size = {m_width, m_height};
    POINT destPoint = {m_rcVirtual.left, m_rcVirtual.top};

    UpdateLayeredWindow(m_hwnd, screenDC, &destPoint, &size, m_memoryDC, &srcPoint, 0, &blend, ULW_ALPHA);

    ReleaseDC(NULL, screenDC);
}

}  // namespace VirtualDesktop