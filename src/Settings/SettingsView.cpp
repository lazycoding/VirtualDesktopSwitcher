#include "SettingsView.h"

#include <CommCtrl.h>
#include <windowsx.h>

#include <string>

namespace VirtualDesktop {

namespace {
constexpr int WINDOW_WIDTH = 400;
constexpr int WINDOW_HEIGHT = 350;
constexpr int CONTROL_MARGIN = 10;
constexpr int CONTROL_HEIGHT = 24;
constexpr int LABEL_WIDTH = 150;
constexpr int CONTROL_WIDTH = 200;
constexpr int COLOR_PREVIEW_SIZE = 24;
}  // namespace

bool SettingsView::show(HINSTANCE hInstance, HWND parentWindow) {
  if (m_hwnd) {
    return true;
  }

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"SettingsView";
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  RegisterClass(&wc);

  m_hwnd = CreateWindowEx(0, L"SettingsView", L"Settings", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, parentWindow,
                          nullptr, hInstance, this);

  if (!m_hwnd) {
    return false;
  }

  // Create controls
  int yPos = CONTROL_MARGIN;

  // Gesture Sensitivity
  CreateWindow(L"STATIC", L"Gesture Sensitivity:", WS_VISIBLE | WS_CHILD, CONTROL_MARGIN, yPos,
               LABEL_WIDTH, CONTROL_HEIGHT, m_hwnd, nullptr, hInstance, nullptr);

  HWND slider = CreateWindow(TRACKBAR_CLASS, nullptr,
                             WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_ENABLESELRANGE,
                             CONTROL_MARGIN + LABEL_WIDTH, yPos, CONTROL_WIDTH, CONTROL_HEIGHT,
                             m_hwnd, nullptr, hInstance, nullptr);
  SendMessage(slider, TBM_SETRANGE, TRUE, MAKELONG(1, 10));
  SendMessage(slider, TBM_SETPOS, TRUE, 5);

  yPos += CONTROL_HEIGHT + CONTROL_MARGIN;

  // Overlay Color
  CreateWindow(L"STATIC", L"Overlay Color:", WS_VISIBLE | WS_CHILD, CONTROL_MARGIN, yPos,
               LABEL_WIDTH, CONTROL_HEIGHT, m_hwnd, nullptr, hInstance, nullptr);

  HWND colorButton = CreateWindow(L"BUTTON", L"Change Color", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  CONTROL_MARGIN + LABEL_WIDTH, yPos, CONTROL_WIDTH, CONTROL_HEIGHT,
                                  m_hwnd, nullptr, hInstance, nullptr);

  yPos += CONTROL_HEIGHT + CONTROL_MARGIN;

  // Color preview
  m_colorPreview = CreateWindow(L"STATIC", nullptr, WS_VISIBLE | WS_CHILD | SS_SUNKEN,
                                CONTROL_MARGIN + LABEL_WIDTH, yPos, COLOR_PREVIEW_SIZE,
                                COLOR_PREVIEW_SIZE, m_hwnd, nullptr, hInstance, nullptr);

  // Update preview with default color
  HBRUSH hBrush = CreateSolidBrush(m_currentColor);
  SetClassLongPtr(m_colorPreview, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
  InvalidateRect(m_colorPreview, nullptr, TRUE);

  return true;
}

void SettingsView::hide() {
  if (m_hwnd) {
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
  }
}

int SettingsView::getGestureSensitivity() const {
  if (!m_hwnd) return 5;  // Default value
  HWND slider = FindWindowEx(m_hwnd, nullptr, TRACKBAR_CLASS, nullptr);
  return (int)SendMessage(slider, TBM_GETPOS, 0, 0);
}

std::wstring SettingsView::getOverlayColor() const {
  wchar_t colorStr[10];
  swprintf_s(colorStr, L"#%02X%02X%02XFF", GetRValue(m_currentColor), GetGValue(m_currentColor),
             GetBValue(m_currentColor));
  return colorStr;
}

LRESULT CALLBACK SettingsView::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_CREATE) {
    CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
    return 0;
  }

  SettingsView *pThis = reinterpret_cast<SettingsView *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (!pThis) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  switch (uMsg) {
    case WM_CLOSE:
      pThis->hide();
      return 0;
    case WM_DESTROY:
      return 0;
    case WM_COMMAND: {
      if (HIWORD(wParam) == BN_CLICKED) {
        // Handle color button click
        CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
        cc.hwndOwner = hwnd;
        cc.lpCustColors = (LPDWORD)pThis->m_customColors;
        cc.rgbResult = pThis->m_currentColor;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;

        if (ChooseColor(&cc)) {
          pThis->m_currentColor = cc.rgbResult;
          // Update color preview
          HBRUSH hBrush = CreateSolidBrush(pThis->m_currentColor);
          SetClassLongPtr(pThis->m_colorPreview, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
          InvalidateRect(pThis->m_colorPreview, nullptr, TRUE);
        }
      }
      return 0;
    }
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

void SettingsView::loadSettings(const Settings &settings) {
  if (!m_hwnd) return;

  // Load gesture sensitivity
  HWND slider = FindWindowEx(m_hwnd, nullptr, TRACKBAR_CLASS, nullptr);
  if (slider) {
    SendMessage(slider, TBM_SETPOS, TRUE, settings.getGestureSensitivity());
  }

  // Load overlay color
  std::wstring colorStr = settings.getOverlayColor();
  if (colorStr.size() >= 7 && colorStr[0] == L'#') {
    int r, g, b;
    swscanf_s(colorStr.c_str() + 1, L"%02x%02x%02x", &r, &g, &b);
    m_currentColor = RGB(r, g, b);

    if (m_colorPreview) {
      HBRUSH hBrush = CreateSolidBrush(m_currentColor);
      SetClassLongPtr(m_colorPreview, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
      InvalidateRect(m_colorPreview, nullptr, TRUE);
    }
  }
}

void SettingsView::saveSettings(Settings &settings) const {
  if (!m_hwnd) return;

  // Save gesture sensitivity
  settings.setGestureSensitivity(getGestureSensitivity());

  // Save overlay color
  settings.setOverlayColor(getOverlayColor());
}

}  // namespace VirtualDesktop