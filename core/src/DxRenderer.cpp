#include "DxRenderer.h"
#include <algorithm>
#include <chrono>

#include <ShellScalingApi.h>
#include <minwinbase.h>
#include <winuser.h>

namespace VirtualDesktop {

namespace {
const D2D1_COLOR_F DEFAULT_TRAIL_COLOR = D2D1::ColorF(D2D1::ColorF::SkyBlue, 0.9f);

bool parseHexComponent(const std::string& hex, size_t offset, float& result) {
    try {
        result = static_cast<float>(std::stoul(hex.substr(offset, 2), nullptr, 16)) / 255.0f;
        return true;
    } catch (...) {
        return false;
    }
}
}  // namespace

DxRenderer::DxRenderer() {
}

DxRenderer::~DxRenderer() {
    destroyDIBAndRenderTarget();
    if (m_factory) {
        m_factory->Release();
    }
}

D2D1_COLOR_F DxRenderer::hexToColorF(const std::string& hex) {
    if (hex.length() != 9 || hex[0] != '#') {
        return DEFAULT_TRAIL_COLOR;
    }

    float r, g, b, a;
    if (parseHexComponent(hex, 1, r) && parseHexComponent(hex, 3, g) && parseHexComponent(hex, 5, b) &&
        parseHexComponent(hex, 7, a)) {
        return D2D1::ColorF(r, g, b, a);
    }
    return DEFAULT_TRAIL_COLOR;
}

void DxRenderer::setTrailStyle(const std::string& colorHex, float lineWidth) {
    m_trailColor = hexToColorF(colorHex);
    m_lineWidth = std::clamp(lineWidth, 1.0f, 10.0f);
    if (m_brush) {
        m_brush->SetColor(m_trailColor);
    }
}

BOOL CALLBACK DxRenderer::monitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM data) {
    UNREFERENCED_PARAMETER(hMonitor);
    UNREFERENCED_PARAMETER(hdc);

    RECT* r = reinterpret_cast<RECT*>(data);
    UnionRect(r, r, lprc);
    return TRUE;
}

void DxRenderer::computeVirtualScreenRect() {
    // Use virtual screen metrics for multi-monitor support
    m_rcVirtual.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_rcVirtual.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_rcVirtual.right = m_rcVirtual.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_rcVirtual.bottom = m_rcVirtual.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_width = std::max((LONG)1, m_rcVirtual.right - m_rcVirtual.left);
    m_height = std::max((LONG)1, m_rcVirtual.bottom - m_rcVirtual.top);

    // Virtual screen coordinates calculated for multi-monitor support
}

bool DxRenderer::initialize(HWND hwndParent) {
    m_hwnd = hwndParent;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory))) {
        return false;
    }

    computeVirtualScreenRect();

    createDIBAndRenderTarget();
    return true;
}

void DxRenderer::resizeForMonitors() {
    destroyDIBAndRenderTarget();
    computeVirtualScreenRect();
    createDIBAndRenderTarget();
}

void DxRenderer::createDIBAndRenderTarget() {
    destroyDIBAndRenderTarget();

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

    HDC hdcScreen = GetDC(NULL);
    m_hDIB = CreateDIBSection(hdcScreen, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &m_pBits, NULL, 0);
    m_memDC = CreateCompatibleDC(hdcScreen);
    // Only select if m_hDIB is valid
    if (m_hDIB) {
        SelectObject(m_memDC, m_hDIB);
    }
    ReleaseDC(NULL, hdcScreen);

    // Get current DPI for render target
    const float dpiX = 96.0f, dpiY = 96.0f;
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpiX,
            dpiY,
            D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

    if (SUCCEEDED(m_factory->CreateDCRenderTarget(&props, &m_renderTarget))) {
        if (m_brush == nullptr)
            m_renderTarget->CreateSolidColorBrush(m_trailColor, &m_brush);
        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND, 1.0f);
        if (m_stroke == nullptr)
            m_factory->CreateStrokeStyle(&strokeProps, nullptr, 0, &m_stroke);
        // Create reusable geometry if not present
        if (m_geometry == nullptr) {
            m_factory->CreatePathGeometry(&m_geometry);
        }
    }
}

void DxRenderer::destroyDIBAndRenderTarget() {
    if (m_brush) {
        m_brush->Release();
        m_brush = nullptr;
    }
    if (m_stroke) {
        m_stroke->Release();
        m_stroke = nullptr;
    }
    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = nullptr;
    }
    if (m_geometry) {
        m_geometry->Release();
        m_geometry = nullptr;
    }
    if (m_memDC) {
        DeleteDC(m_memDC);
        m_memDC = nullptr;
    }
    if (m_hDIB) {
        DeleteObject(m_hDIB);
        m_hDIB = nullptr;
        m_pBits = nullptr;
    }
}

void DxRenderer::precomputeBezierTable() {
    for (int i = 0; i < BEZIER_TABLE_SIZE; ++i) {
        float t = static_cast<float>(i) / (BEZIER_TABLE_SIZE - 1);
        s_bezierTable[i] = t * t;  // Precompute t^2 for quadratic bezier
    }
}

DxRenderer::FPoint DxRenderer::quadraticBezier(const FPoint& p0, const FPoint& p1, const FPoint& p2, float t) {
    // Quadratic Bezier formula: B(t) = (1-t)^2 * P0 +2*(1-t)*t * P1 + t^2 * P2
    float oneMinusT = 1.0f - t;
    float oneMinusTSquared = oneMinusT * oneMinusT;
    float tSquared = t * t;

    return {oneMinusTSquared * p0.x + 2 * oneMinusT * t * p1.x + tSquared * p2.x,
            oneMinusTSquared * p0.y + 2 * oneMinusT * t * p1.y + tSquared * p2.y};
}

void DxRenderer::render(const std::vector<POINT>& points) {
    if (!m_renderTarget || points.size() < 2) {
        return;
    }

    // Throttle renders to ~60 FPS to avoid excessive CPU/GPU load
    static auto lastRender = std::chrono::steady_clock::time_point::min();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRender).count();
    if (elapsed < 16) {
        // Skip this frame if it's too soon
        return;
    }
    lastRender = now;

    // Convert screen coordinates to window-relative physical pixel coordinates
    std::vector<FPoint> fpts;
    fpts.reserve(points.size());

    for (auto& p : points) {
        fpts.push_back({(float)(p.x - m_rcVirtual.left), (float)(p.y - m_rcVirtual.top)});
    }

    RECT bind = {0, 0, m_width, m_height};
    if (FAILED(m_renderTarget->BindDC(m_memDC, &bind))) {
        return;
    }

    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2D1::ColorF(0, 0.0f));

    // Initialize bezier table on first use
    static bool bezierTableInitialized = false;
    if (!bezierTableInitialized) {
        precomputeBezierTable();
        bezierTableInitialized = true;
    }

    // Set high-quality rendering options
    m_renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    m_renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    // Reuse geometry: populate m_geometry's sink only if point count changed significantly
    ID2D1PathGeometry* geo = m_geometry;
    ID2D1GeometrySink* sink = nullptr;
    if (geo) {
        if (SUCCEEDED(geo->Open(&sink))) {
            sink->BeginFigure(D2D1::Point2F(fpts[0].x, fpts[0].y), D2D1_FIGURE_BEGIN_HOLLOW);

            if (fpts.size() >= 3) {
                for (size_t i = 0; i + 1 < fpts.size(); i++) {
                    if (i + 2 < fpts.size()) {
                        FPoint controlPoint = fpts[i + 1];
                        sink->AddQuadraticBezier(
                                D2D1::QuadraticBezierSegment(
                                        D2D1::Point2F(controlPoint.x, controlPoint.y),
                                        D2D1::Point2F(fpts[i + 1].x, fpts[i + 1].y)));
                    } else {
                        sink->AddLine(D2D1::Point2F(fpts[i + 1].x, fpts[i + 1].y));
                    }
                }
            } else {
                for (size_t i = 1; i < fpts.size(); i++) {
                    sink->AddLine(D2D1::Point2F(fpts[i].x, fpts[i].y));
                }
            }

            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            sink->Close();
            sink->Release();
            sink = nullptr;
        }
    }

    if (geo) {
        m_brush->SetColor(m_trailColor);
        m_renderTarget->DrawGeometry(geo, m_brush, m_lineWidth, m_stroke);
    }

    HRESULT hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        return;
    }

    HDC hdcScreen = GetDC(NULL);
    POINT ptDst = {m_rcVirtual.left, m_rcVirtual.top};
    SIZE size = {m_width, m_height};
    POINT ptSrc = {0, 0};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    // Update layered window using physical pixel coordinates
    UpdateLayeredWindow(m_hwnd, hdcScreen, &ptDst, &size, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
    ReleaseDC(NULL, hdcScreen);
}

void DxRenderer::clear() {
    if (!m_renderTarget) {
        return;
    }
    RECT bind = {0, 0, m_width, m_height};
    if (FAILED(m_renderTarget->BindDC(m_memDC, &bind))) {
        return;
    }
    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2D1::ColorF(0, 0.0f));
    m_renderTarget->EndDraw();
    HDC hdcScreen = GetDC(NULL);
    POINT ptDst = {m_rcVirtual.left, m_rcVirtual.top};
    SIZE size = {m_width, m_height};
    POINT ptSrc = {0, 0};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(m_hwnd, hdcScreen, &ptDst, &size, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
    ReleaseDC(NULL, hdcScreen);
}

}  // namespace VirtualDesktop