#include "../include/OverlayUI.h"
#include <algorithm>
#include <wincodec.h>

// 窗口类名
const wchar_t OVERLAY_WINDOW_CLASS[] = L"VirtualDesktopSwitcherOverlay";

bool OverlayUI::Initialize() {
  // 创建D2D工厂
  if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) {
    return false;
  }

  // 创建透明窗口
  WNDCLASSEXW wcex = {sizeof(WNDCLASSEXW)};
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = GetModuleHandle(nullptr);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.lpszClassName = OVERLAY_WINDOW_CLASS;
  RegisterClassExW(&wcex);

  hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
                         OVERLAY_WINDOW_CLASS, L"DesktopSwitcherOverlay",
                         WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                         GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr,
                         GetModuleHandle(nullptr), this);

  if (!hWnd)
    return false;

  // 设置窗口透明
  SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY | LWA_ALPHA);

  // 显示窗口
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);

  return CreateDeviceResources();
}

void OverlayUI::Shutdown() {
  DiscardDeviceResources();
  if (hWnd)
    DestroyWindow(hWnd);
  if (pFactory)
    pFactory->Release();
}

void OverlayUI::AddTrailPoint(POINT pt) {
  if (trailSegments.empty()) {
    trailSegments.push_back({D2D1::Point2F((float)pt.x, (float)pt.y),
                             D2D1::Point2F((float)pt.x, (float)pt.y), 1.0f});
  } else {
    const auto &last = trailSegments.back();
    trailSegments.push_back(
        {last.end, D2D1::Point2F((float)pt.x, (float)pt.y), 1.0f});
  }
  Render();
}

void OverlayUI::Render() {
  if (!pRenderTarget)
    return;

  pRenderTarget->BeginDraw();
  pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));

  // 绘制轨迹
  for (auto &segment : trailSegments) {
    pBrush->SetOpacity(segment.opacity);
    pRenderTarget->DrawLine(segment.start, segment.end, pBrush, lineWidth);

    // 更新透明度
    segment.opacity = std::max(0.0f, segment.opacity - fadeSpeed);
  }

  // 移除完全透明的线段
  trailSegments.erase(
      std::remove_if(trailSegments.begin(), trailSegments.end(),
                     [](const LineSegment &s) { return s.opacity <= 0.0f; }),
      trailSegments.end());

  pRenderTarget->EndDraw();
}

void OverlayUI::Clear() {
  trailSegments.clear();
  if (pRenderTarget) {
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    pRenderTarget->EndDraw();
  }
}

void OverlayUI::SetLineColor(D2D1::ColorF color) {
  lineColor = color;
  if (pBrush) {
    pBrush->SetColor(lineColor);
  }
}

void OverlayUI::SetLineWidth(float width) {
  lineWidth = std::max(1.0f, std::min(10.0f, width));
}

void OverlayUI::SetFadeSpeed(float speed) {
  fadeSpeed = std::max(0.01f, std::min(0.5f, speed));
}

bool OverlayUI::CreateDeviceResources() {
  if (!pRenderTarget && hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    HRESULT hr = pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, size), &pRenderTarget);

    if (SUCCEEDED(hr)) {
      hr = pRenderTarget->CreateSolidColorBrush(lineColor, &pBrush);
    }

    return SUCCEEDED(hr);
  }
  return false;
}

void OverlayUI::DiscardDeviceResources() {
  if (pBrush)
    pBrush->Release();
  if (pRenderTarget)
    pRenderTarget->Release();
  pBrush = nullptr;
  pRenderTarget = nullptr;
}

LRESULT CALLBACK OverlayUI::WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  if (message == WM_CREATE) {
    LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
    return 0;
  }

  OverlayUI *pThis = (OverlayUI *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (!pThis)
    return DefWindowProc(hWnd, message, wParam, lParam);

  switch (message) {
  case WM_DISPLAYCHANGE:
  case WM_SIZE:
    pThis->DiscardDeviceResources();
    break;

  case WM_DESTROY:
    pThis->DiscardDeviceResources();
    break;

  case WM_PAINT:
    pThis->Render();
    ValidateRect(hWnd, nullptr);
    break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}