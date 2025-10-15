#include "OverlayUI.h"
#include <d2d1_1.h>
#include <wrl/client.h>


using Microsoft::WRL::ComPtr;

namespace VirtualDesktop {

OverlayUI::OverlayUI() = default;

OverlayUI::~OverlayUI() { releaseResources(); }

bool OverlayUI::initialize(HWND hwnd) {
  m_hwnd = hwnd;

  HRESULT hr =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);

  if (FAILED(hr)) {
    return false;
  }

  RECT rc;
  GetClientRect(m_hwnd, &rc);

  hr = m_d2dFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(
          m_hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
      &m_renderTarget);

  if (FAILED(hr)) {
    return false;
  }

  hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::LightBlue, 0.7f), &m_brush);

  return SUCCEEDED(hr);
}

void OverlayUI::render(const std::vector<POINT> &points) {
  if (!m_renderTarget || points.size() < 2) {
    return;
  }

  m_renderTarget->BeginDraw();
  m_renderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));

  for (size_t i = 1; i < points.size(); ++i) {
    m_renderTarget->DrawLine(D2D1::Point2F(static_cast<float>(points[i - 1].x),
                                           static_cast<float>(points[i - 1].y)),
                             D2D1::Point2F(static_cast<float>(points[i].x),
                                           static_cast<float>(points[i].y)),
                             m_brush, 3.0f);
  }

  m_renderTarget->EndDraw();
}

void OverlayUI::clear() {
  if (m_renderTarget) {
    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
    m_renderTarget->EndDraw();
  }
}

void OverlayUI::releaseResources() {
  if (m_brush) {
    m_brush->Release();
    m_brush = nullptr;
  }
  if (m_renderTarget) {
    m_renderTarget->Release();
    m_renderTarget = nullptr;
  }
  if (m_d2dFactory) {
    m_d2dFactory->Release();
    m_d2dFactory = nullptr;
  }
}

} // namespace VirtualDesktop