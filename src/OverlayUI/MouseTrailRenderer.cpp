#include "MouseTrailRenderer.h"

#include <algorithm>
#include <cwchar>

MouseTrailRenderer::MouseTrailRenderer() {}
MouseTrailRenderer::~MouseTrailRenderer() {
  DestroyDIBAndRenderTarget();
  if (m_factory) m_factory->Release();
}

D2D1_COLOR_F MouseTrailRenderer::HexToColorF(const std::wstring& hex) {
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
  return D2D1::ColorF(static_cast<float>(rValue) / 255.0f, static_cast<float>(gValue) / 255.0f,
                      static_cast<float>(bValue) / 255.0f, static_cast<float>(aValue) / 255.0f);
}
void MouseTrailRenderer::SetTrailStyle(const std::wstring& colorHex, float lineWidth) {
  m_trailColor = HexToColorF(colorHex);
  m_lineWidth = std::clamp(lineWidth, 1.0f, 10.0f);
}

BOOL CALLBACK MouseTrailRenderer::MonitorEnumProc(HMONITOR, HDC, LPRECT lprc, LPARAM data) {
  RECT* r = reinterpret_cast<RECT*>(data);
  UnionRect(r, r, lprc);
  return TRUE;
}

void MouseTrailRenderer::ComputeVirtualScreenRect() {
  RECT r = {0, 0, 0, 0};
  EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&r);
  if (IsRectEmpty(&r)) {
    r.left = 0;
    r.top = 0;
    r.right = GetSystemMetrics(SM_CXSCREEN);
    r.bottom = GetSystemMetrics(SM_CYSCREEN);
  }
  m_rcVirtual = r;
  m_width = std::max((LONG)1, r.right - r.left);
  m_height = std::max((LONG)1, r.bottom - r.top);
}

bool MouseTrailRenderer::Initialize(HWND hwndParent) {
  m_hwnd = hwndParent;
  if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory))) return false;

  ComputeVirtualScreenRect();

  // 设置hwnd的size为m_rcVirtual
  SetWindowPos(m_hwnd, NULL, m_rcVirtual.left, m_rcVirtual.top, m_width, m_height,
               SWP_NOACTIVATE | SWP_NOZORDER);

  CreateDIBAndRenderTarget();
  return true;
}

void MouseTrailRenderer::ResizeForMonitors() {
  DestroyDIBAndRenderTarget();
  ComputeVirtualScreenRect();
  CreateDIBAndRenderTarget();
}

void MouseTrailRenderer::CreateDIBAndRenderTarget() {
  DestroyDIBAndRenderTarget();

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
  SelectObject(m_memDC, m_hDIB);
  ReleaseDC(NULL, hdcScreen);

  D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 96.0f, 96.0f,
      D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

  if (SUCCEEDED(m_factory->CreateDCRenderTarget(&props, &m_rt))) {
    m_rt->CreateSolidColorBrush(m_trailColor, &m_brush);
    D2D1_STROKE_STYLE_PROPERTIES strokeProps =
        D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                    D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND, 1.0f);
    m_factory->CreateStrokeStyle(&strokeProps, nullptr, 0, &m_stroke);
  }
}

void MouseTrailRenderer::DestroyDIBAndRenderTarget() {
  if (m_brush) {
    m_brush->Release();
    m_brush = nullptr;
  }
  if (m_stroke) {
    m_stroke->Release();
    m_stroke = nullptr;
  }
  if (m_rt) {
    m_rt->Release();
    m_rt = nullptr;
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

MouseTrailRenderer::FPoint MouseTrailRenderer::CatmullRom(const FPoint& p0, const FPoint& p1,
                                                          const FPoint& p2, const FPoint& p3,
                                                          float t) {
  float t2 = t * t;
  float t3 = t2 * t;
  return {0.5f * ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
                  (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3),
          0.5f * ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
                  (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3)};
}

void MouseTrailRenderer::Render(const std::vector<POINT>& pts) {
  if (!m_rt || pts.size() < 4) return;

  // 转换点为虚拟屏坐标
  std::vector<FPoint> fpts;
  fpts.reserve(pts.size());
  for (auto& p : pts)
    fpts.push_back({(float)(p.x - m_rcVirtual.left), (float)(p.y - m_rcVirtual.top)});

  RECT bind = {0, 0, m_width, m_height};
  if (FAILED(m_rt->BindDC(m_memDC, &bind))) return;

  m_rt->BeginDraw();
  m_rt->Clear(D2D1::ColorF(0, 0.0f));

  ID2D1PathGeometry* geo = nullptr;
  if (SUCCEEDED(m_factory->CreatePathGeometry(&geo))) {
    ID2D1GeometrySink* sink = nullptr;
    if (SUCCEEDED(geo->Open(&sink))) {
      D2D1_POINT_2F start = D2D1::Point2F(fpts[1].x, fpts[1].y);
      sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);

      for (size_t i = 1; i + 2 < fpts.size(); ++i) {
        for (int s = 1; s <= m_steps; ++s) {
          float t = (float)s / (float)m_steps;
          auto q = CatmullRom(fpts[i - 1], fpts[i], fpts[i + 1], fpts[i + 2], t);
          sink->AddLine(D2D1::Point2F(q.x, q.y));
        }
      }
      sink->EndFigure(D2D1_FIGURE_END_OPEN);
      sink->Close();
      sink->Release();

      m_brush->SetColor(m_trailColor);
      m_rt->DrawGeometry(geo, m_brush, m_lineWidth, m_stroke);
    }
    geo->Release();
  }

  m_rt->EndDraw();

  HDC hdcScreen = GetDC(NULL);
  POINT ptDst = {m_rcVirtual.left, m_rcVirtual.top};
  SIZE size = {m_width, m_height};
  POINT ptSrc = {0, 0};
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  UpdateLayeredWindow(m_hwnd, hdcScreen, &ptDst, &size, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
  ReleaseDC(NULL, hdcScreen);
}

void MouseTrailRenderer::Clear() {
  if (!m_rt) return;
  RECT bind = {0, 0, m_width, m_height};
  if (FAILED(m_rt->BindDC(m_memDC, &bind))) return;
  m_rt->BeginDraw();
  m_rt->Clear(D2D1::ColorF(0, 0.0f));
  m_rt->EndDraw();
  HDC hdcScreen = GetDC(NULL);
  POINT ptDst = {m_rcVirtual.left, m_rcVirtual.top};
  SIZE size = {m_width, m_height};
  POINT ptSrc = {0, 0};
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  UpdateLayeredWindow(m_hwnd, hdcScreen, &ptDst, &size, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
  ReleaseDC(NULL, hdcScreen);
}
