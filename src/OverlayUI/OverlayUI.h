#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <vector>

namespace VirtualDesktop {

/**
 * @brief Renders overlay UI for gesture visualization
 */
class OverlayUI {
public:
  OverlayUI();
  ~OverlayUI();

  /**
   * @brief Initializes Direct2D resources
   * @param hwnd Parent window handle
   * @return true if initialization succeeded
   */
  bool initialize(HWND hwnd);

  /**
   * @brief Renders the gesture path
   * @param points Vector of mouse positions
   */
  void render(const std::vector<POINT> &points);

  /**
   * @brief Clears the overlay
   */
  void clear();

private:
  HWND m_hwnd = nullptr;
  ID2D1Factory *m_d2dFactory = nullptr;
  ID2D1HwndRenderTarget *m_renderTarget = nullptr;
  ID2D1SolidColorBrush *m_brush = nullptr;

  void releaseResources();
};

} // namespace VirtualDesktop