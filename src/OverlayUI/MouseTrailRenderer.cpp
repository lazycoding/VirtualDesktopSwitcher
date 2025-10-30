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
    if (m_hwnd) {
        // Use the monitor where the window is located
        HMONITOR hMonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        monitorInfo.rcWork = {0, 0, 0, 0};
        monitorInfo.dwFlags = 0;
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
            // Use the monitor's work area (excluding taskbar)
            m_rcVirtual = monitorInfo.rcWork;
            m_width = std::max((LONG)1, m_rcVirtual.right - m_rcVirtual.left);
            m_height = std::max((LONG)1, m_rcVirtual.bottom - m_rcVirtual.top);
            trace("monitor rect: [%d, %d, %d, %d]",
                  m_rcVirtual.left,
                  m_rcVirtual.top,
                  m_rcVirtual.right,
                  m_rcVirtual.bottom);
            trace("monitor size: %d x %d", m_width, m_height);
            return;
        }
    }

    // Fallback to primary monitor
    RECT r = {0, 0, 0, 0};
    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)&r);
    if (IsRectEmpty(&r)) {
        r.left = 0;
        r.top = 0;
        r.right = GetSystemMetrics(SM_CXSCREEN);
        r.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    m_rcVirtual = r;
    m_width = std::max((LONG)1, r.right - r.left);
    m_height = std::max((LONG)1, r.bottom - r.top);
    trace("fallback rect: [%d, %d, %d, %d]", r.left, r.top, r.right, r.bottom);
    trace("fallback size: %d x %d", m_width, m_height);
}

bool MouseTrailRenderer::initialize(HWND hwndParent) {
    m_hwnd = hwndParent;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory))) {
        return false;
    }

    computeVirtualScreenRect();

    // Set window size to m_rcVirtual
    SetWindowPos(m_hwnd, NULL, m_rcVirtual.left, m_rcVirtual.top, m_width, m_height, SWP_NOACTIVATE | SWP_NOZORDER);

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

MouseTrailRenderer::FPoint MouseTrailRenderer::catmullRom(
        const FPoint& p0,
        const FPoint& p1,
        const FPoint& p2,
        const FPoint& p3,
        float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return {0.5f *
                    ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
                     (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3),
            0.5f *
                    ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
                     (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3)};
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

    ID2D1PathGeometry* geo = nullptr;
    if (points.size() >= 4) {
        // Use Catmull-Rom spline for smooth rendering (requires at least 4 points)
        if (SUCCEEDED(m_factory->CreatePathGeometry(&geo))) {
            ID2D1GeometrySink* sink = nullptr;
            if (SUCCEEDED(geo->Open(&sink))) {
                D2D1_POINT_2F start = D2D1::Point2F(fpts[1].x, fpts[1].y);
                sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);

                for (size_t i = 1; i + 2 < fpts.size(); ++i) {
                    for (int s = 1; s <= m_steps; ++s) {
                        float t = (float)s / (float)m_steps;
                        auto q = catmullRom(fpts[i - 1], fpts[i], fpts[i + 1], fpts[i + 2], t);
                        sink->AddLine(D2D1::Point2F(q.x, q.y));
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
    } else if (points.size() >= 2) {
        // For 2-3 points, use simple line connections
        if (SUCCEEDED(m_factory->CreatePathGeometry(&geo))) {
            ID2D1GeometrySink* sink = nullptr;
            if (SUCCEEDED(geo->Open(&sink))) {
                sink->BeginFigure(D2D1::Point2F(fpts[0].x, fpts[0].y), D2D1_FIGURE_BEGIN_HOLLOW);

                for (size_t i = 1; i < fpts.size(); ++i) {
                    sink->AddLine(D2D1::Point2F(fpts[i].x, fpts[i].y));
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