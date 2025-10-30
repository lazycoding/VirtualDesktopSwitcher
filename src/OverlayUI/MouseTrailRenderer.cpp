#include "OverlayUI/MouseTrailRenderer.h"
#include "utils.h"
#include <algorithm>

#include <ShellScalingApi.h>
#include <minwinbase.h>
#include <winuser.h>

namespace VirtualDesktop {

MouseTrailRenderer::MouseTrailRenderer() {
}

MouseTrailRenderer::~MouseTrailRenderer() {
    destroyDIBAndRenderTarget();
    if (m_factory) {
        m_factory->Release();
    }
}

D2D1_COLOR_F MouseTrailRenderer::hexToColorF(const std::wstring& hex) {
    if (hex.length() != 9 || hex[0] != L'#') {
        // Return default color if invalid format
        return D2D1::ColorF(D2D1::ColorF::SkyBlue, 0.9f);
    }

    // Parse RR, GG, BB, AA components
    wchar_t* end;
    std::wstring r = hex.substr(1, 2);
    std::wstring g = hex.substr(3, 2);
    std::wstring b = hex.substr(5, 2);
    std::wstring a = hex.substr(7, 2);

    long rValue = std::wcstol(r.c_str(), &end, 16);
    long gValue = std::wcstol(g.c_str(), &end, 16);
    long bValue = std::wcstol(b.c_str(), &end, 16);
    long aValue = std::wcstol(a.c_str(), &end, 16);

    // Convert to 0.0-1.0 range
    return D2D1::ColorF(
            static_cast<float>(rValue) / 255.0f,
            static_cast<float>(gValue) / 255.0f,
            static_cast<float>(bValue) / 255.0f,
            static_cast<float>(aValue) / 255.0f);
}

void MouseTrailRenderer::setTrailStyle(const std::wstring& colorHex, float lineWidth) {
    m_trailColor = hexToColorF(colorHex);
    m_lineWidth = std::clamp(lineWidth, 1.0f, 10.0f);
}

BOOL CALLBACK MouseTrailRenderer::monitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM data) {
    UNREFERENCED_PARAMETER(hMonitor);
    UNREFERENCED_PARAMETER(hdc);

    RECT* r = reinterpret_cast<RECT*>(data);
    UnionRect(r, r, lprc);
    return TRUE;
}

void MouseTrailRenderer::computeVirtualScreenRect() {
    // Use virtual screen metrics for multi-monitor support
    m_rcVirtual.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_rcVirtual.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_rcVirtual.right = m_rcVirtual.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_rcVirtual.bottom = m_rcVirtual.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_width = std::max((LONG)1, m_rcVirtual.right - m_rcVirtual.left);
    m_height = std::max((LONG)1, m_rcVirtual.bottom - m_rcVirtual.top);

    // Virtual screen coordinates calculated for multi-monitor support
}

bool MouseTrailRenderer::initialize(HWND hwndParent) {
    m_hwnd = hwndParent;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory))) {
        return false;
    }

    computeVirtualScreenRect();

    createDIBAndRenderTarget();
    return true;
}

void MouseTrailRenderer::resizeForMonitors() {
    destroyDIBAndRenderTarget();
    computeVirtualScreenRect();
    createDIBAndRenderTarget();
}

void MouseTrailRenderer::createDIBAndRenderTarget() {
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
        m_renderTarget->CreateSolidColorBrush(m_trailColor, &m_brush);
        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND, 1.0f);
        m_factory->CreateStrokeStyle(&strokeProps, nullptr, 0, &m_stroke);
    }
}

void MouseTrailRenderer::destroyDIBAndRenderTarget() {
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

void MouseTrailRenderer::precomputeBezierTable() {
    for (int i = 0; i < BEZIER_TABLE_SIZE; ++i) {
        float t = static_cast<float>(i) / (BEZIER_TABLE_SIZE - 1);
        s_bezierTable[i] = t * t;  // Precompute t^2 for quadratic bezier
    }
}

MouseTrailRenderer::FPoint MouseTrailRenderer::quadraticBezier(
        const FPoint& p0,
        const FPoint& p1,
        const FPoint& p2,
        float t) {
    // Quadratic Bezier formula: B(t) = (1-t)^2 * P0 + 2*(1-t)*t * P1 + t^2 * P2
    float oneMinusT = 1.0f - t;
    float oneMinusTSquared = oneMinusT * oneMinusT;
    float tSquared = t * t;

    return {oneMinusTSquared * p0.x + 2 * oneMinusT * t * p1.x + tSquared * p2.x,
            oneMinusTSquared * p0.y + 2 * oneMinusT * t * p1.y + tSquared * p2.y};
}

void MouseTrailRenderer::render(const std::vector<POINT>& points) {
    if (!m_renderTarget || points.size() < 2) {
        return;
    }

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

    // Create geometry for rendering
    ID2D1PathGeometry* geo = nullptr;
    if (points.size() >= 3) {
        // Use quadratic Bezier curves for smooth rendering (requires at least 3 points)
        if (SUCCEEDED(m_factory->CreatePathGeometry(&geo))) {
            ID2D1GeometrySink* sink = nullptr;
            if (SUCCEEDED(geo->Open(&sink))) {
                // Start from first point
                sink->BeginFigure(D2D1::Point2F(fpts[0].x, fpts[0].y), D2D1_FIGURE_BEGIN_HOLLOW);

                // Use quadratic Bezier curves between points
                for (size_t i = 0; i + 2 < fpts.size(); i += 1) {
                    const FPoint& p0 = fpts[i];
                    const FPoint& p1 = fpts[i + 1];
                    const FPoint& p2 = fpts[i + 2];

                    // Control point is the midpoint between p0 and p1
                    FPoint controlPoint = {(p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f};

                    // Add quadratic Bezier curve from p0 to p1 with control point
                    if (i == 0) {
                        sink->AddLine(D2D1::Point2F(p0.x, p0.y));
                    }
                    sink->AddQuadraticBezier(
                            D2D1::QuadraticBezierSegment(
                                    D2D1::Point2F(controlPoint.x, controlPoint.y), D2D1::Point2F(p1.x, p1.y)));

                    // For the last segment, add final curve to p2
                    if (i + 3 >= fpts.size()) {
                        FPoint finalControlPoint = {(p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f};
                        sink->AddQuadraticBezier(
                                D2D1::QuadraticBezierSegment(
                                        D2D1::Point2F(finalControlPoint.x, finalControlPoint.y),
                                        D2D1::Point2F(p2.x, p2.y)));
                        break;
                    }
                }

                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                sink->Close();
                sink->Release();

                m_brush->SetColor(m_trailColor);
                m_renderTarget->DrawGeometry(geo, m_brush, m_lineWidth, m_stroke);
            }
            geo->Release();
        }
    }

    // Handle simple line connections for 2 points
    if (points.size() == 2) {
        ID2D1PathGeometry* lineGeo = nullptr;
        if (SUCCEEDED(m_factory->CreatePathGeometry(&lineGeo))) {
            ID2D1GeometrySink* sink = nullptr;
            if (SUCCEEDED(lineGeo->Open(&sink))) {
                sink->BeginFigure(D2D1::Point2F(fpts[0].x, fpts[0].y), D2D1_FIGURE_BEGIN_HOLLOW);
                sink->AddLine(D2D1::Point2F(fpts[1].x, fpts[1].y));
                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                sink->Close();
                sink->Release();

                m_brush->SetColor(m_trailColor);
                m_renderTarget->DrawGeometry(lineGeo, m_brush, m_lineWidth, m_stroke);
            }
            lineGeo->Release();
        }
    }

    m_renderTarget->EndDraw();

    HDC hdcScreen = GetDC(NULL);
    POINT ptDst = {m_rcVirtual.left, m_rcVirtual.top};
    SIZE size = {m_width, m_height};
    POINT ptSrc = {0, 0};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    // Update layered window using physical pixel coordinates
    UpdateLayeredWindow(m_hwnd, hdcScreen, &ptDst, &size, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
    ReleaseDC(NULL, hdcScreen);
}

void MouseTrailRenderer::clear() {
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