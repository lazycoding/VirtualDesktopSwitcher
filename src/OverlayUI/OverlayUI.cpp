#include "OverlayUI.h"
#include <d2d1_1.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace VirtualDesktop {

    OverlayUI::OverlayUI() : m_settings(nullptr) {}

    OverlayUI::~OverlayUI() {}

    /**
     * @brief Initializes the overlay window with transparent background and
     * fullscreen size
     * @return true if initialization succeeded, false otherwise
     */
    bool OverlayUI::initialize(HINSTANCE hInst) {
        // Register window class
        const wchar_t className[] = L"VirtualDesktopOverlayClass";
        WNDCLASS windowClass = {};
        windowClass.lpfnWndProc = DefWindowProc;
        windowClass.hInstance = GetModuleHandle(nullptr);
        windowClass.lpszClassName = className;
        windowClass.lpfnWndProc = WinProc;
        if (!RegisterClass(&windowClass)) {
            return false;
        }

        // Create window with transparent properties
        DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
            WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
        m_hWnd = CreateWindowExW(ex, className, L"", WS_POPUP,
            0, 0, CW_DEFAULT, CW_DEFAULT,
            NULL, NULL, hInst, this);

        // Apply settings to the mouse trail renderer if settings are available
        if (m_settings) {
            std::wstring colorHex = m_settings->getOverlayColor();
            int sensitivity = m_settings->getGestureSensitivity();
            // Convert sensitivity to line width (1-10 sensitivity to 1-10 line width)
            float lineWidth = static_cast<float>(sensitivity * 2); // Scale as needed
            m_mouseTrailRenderer.SetTrailStyle(colorHex, lineWidth);
        }

        m_mouseTrailRenderer.Initialize(m_hWnd);
        return true;
    }

    void OverlayUI::setSettings(const Settings& settings) {
        m_settings = &settings;
        
        // Apply settings to the mouse trail renderer
        std::wstring colorHex = m_settings->getOverlayColor();
        int sensitivity = m_settings->getGestureSensitivity();
        // Convert sensitivity to line width (1-10 sensitivity to 1-10 line width)
        float lineWidth = static_cast<float>(sensitivity * 2); // Scale as needed
        m_mouseTrailRenderer.SetTrailStyle(colorHex, lineWidth);
    }

    void OverlayUI::clear() { m_mouseTrailRenderer.Clear(); }

    LRESULT OverlayUI::WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE) {
            // Store the OverlayUI instance in window user data
            CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
            OverlayUI* pOverlay = (OverlayUI*)pcs->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pOverlay);
            return 0;
        }
        else {
            // Load the OverlayUI instance from window user data
            OverlayUI* pOverlay = (OverlayUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            switch (message)
            {
            case WM_DISPLAYCHANGE:
            case WM_SETTINGCHANGE:
            {
                if (pOverlay)
                {
                    pOverlay->m_mouseTrailRenderer.ResizeForMonitors();
                }
                break;
            }
            default:
                break;
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    void OverlayUI::show() {
        if (m_hWnd != nullptr) {
            ShowWindow(m_hWnd, SW_SHOW);
            UpdateWindow(m_hWnd);
        }
    }

    void OverlayUI::hide() {
        if (m_hWnd != nullptr) {
            ShowWindow(m_hWnd, SW_HIDE);
            // Clear trajectory points when hiding
            m_trajectoryPoints.clear();
            // Clear the overlay display
            clear();
        }
    }

    void OverlayUI::updatePosition(int x, int y) {
        if (m_hWnd == nullptr) {
            return;
        }

        // Add new point to trajectory
        POINT newPoint = { x, y };
        m_trajectoryPoints.push_back(newPoint);

        // Render the updated trajectory
        render(m_trajectoryPoints);
    }

    void OverlayUI::render(const std::vector<POINT>& points) {
        if (m_hWnd == nullptr) {
            return;
        }
        m_mouseTrailRenderer.Render(points);
    }

} // namespace VirtualDesktop