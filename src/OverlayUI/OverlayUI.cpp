#include "OverlayUI/OverlayUI.h"
#include "utils.h"
#include <d2d1_1.h>
#include <wrl/client.h>
#include <cmath>

namespace VirtualDesktop {

OverlayUI::OverlayUI() : m_settings(nullptr), m_useDirect2D(true) {  // Default to Direct2D
}

OverlayUI::~OverlayUI() {
}

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
    windowClass.lpfnWndProc = windowProc;
    if (!RegisterClass(&windowClass)) {
        return false;
    }

    // Create window with transparent properties
    DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

    // Use virtual screen size for multi-monitor support
    int virtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int virtualScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int virtualScreenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int virtualScreenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

    m_hWnd = CreateWindowExW(
            ex,
            className,
            L"",
            WS_POPUP,
            virtualScreenLeft,
            virtualScreenTop,
            virtualScreenWidth,
            virtualScreenHeight,
            NULL,
            NULL,
            hInst,
            this);
    trace("overlay window created at [%d, %d] size %d x %d (virtual screen)",
          virtualScreenLeft,
          virtualScreenTop,
          virtualScreenWidth,
          virtualScreenHeight);
          
    // Determine which renderer to use based on settings
    if (m_settings) {
        switchRenderer();
    } else {
        // Initialize Direct2D by default
        m_useDirect2D = true;
        std::wstring defaultColor = L"#6495EDAA";  // Default color
        float defaultWidth = 5.0f;  // Default width
        m_direct2dRenderer.setTrailStyle(defaultColor, defaultWidth);
        m_direct2dRenderer.initialize(m_hWnd);
    }

    return true;
}

void OverlayUI::switchRenderer() {
    if (!m_settings) {
        return;
    }
    
    std::wstring renderingMode = m_settings->getRenderingMode();
    bool useDirect2D = (renderingMode == L"Direct2D");
    
    if (useDirect2D != m_useDirect2D) {
        // Clear the current renderer before switching
        if (m_useDirect2D) {
            m_direct2dRenderer.clear();
        } else {
            m_gdiRenderer.clear();
        }
        
        m_useDirect2D = useDirect2D;
    }
    
    // Apply settings to the active renderer
    std::wstring colorHex = m_settings->getOverlayColor();
    float lineWidth = static_cast<float>(m_settings->getGestureLineWidth());
    
    if (m_useDirect2D) {
        m_direct2dRenderer.setTrailStyle(colorHex, lineWidth);
        if (!m_direct2dRenderer.initialize(m_hWnd)) {
            // If Direct2D initialization fails, try GDI
            m_useDirect2D = false;
            m_gdiRenderer.setTrailStyle(colorHex, lineWidth);
            m_gdiRenderer.initialize(m_hWnd);
        }
    } else {
        m_gdiRenderer.setTrailStyle(colorHex, lineWidth);
        m_gdiRenderer.initialize(m_hWnd);
    }
}

void OverlayUI::setSettings(const Settings& settings) {
    m_settings = &settings;
    switchRenderer();  // Switch renderer based on new settings
}

void OverlayUI::clear() {
    if (m_useDirect2D) {
        m_direct2dRenderer.clear();
    } else {
        m_gdiRenderer.clear();
    }
}

LRESULT OverlayUI::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) {
        // Store the OverlayUI instance in window user data
        CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
        OverlayUI* pOverlay = (OverlayUI*)pcs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pOverlay);
        return 0;
    } else {
        // Load the OverlayUI instance from window user data
        OverlayUI* pOverlay = (OverlayUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        switch (message) {
            case WM_DISPLAYCHANGE:
            case WM_SETTINGCHANGE: {
                if (pOverlay) {
                    if (pOverlay->m_useDirect2D) {
                        pOverlay->m_direct2dRenderer.resizeForMonitors();
                    } else {
                        pOverlay->m_gdiRenderer.resizeForMonitors();
                    }
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

void OverlayUI::smoothTrajectory() {
    if (m_trajectoryPoints.size() < 2) {
        m_smoothedPoints = m_trajectoryPoints;  // Not enough points to smooth
        return;
    }

    m_smoothedPoints.clear();
    
    // For small number of points, just use original points
    if (m_trajectoryPoints.size() < 3) {
        m_smoothedPoints = m_trajectoryPoints;
        return;
    }
    
    // For smooth trajectories, we'll add intermediate points between each pair of original points
    // This helps make the line appear more continuous
    
    // Copy the first point
    m_smoothedPoints.push_back(m_trajectoryPoints[0]);
    
    for (size_t i = 1; i < m_trajectoryPoints.size(); ++i) {
        const POINT& prevPt = m_trajectoryPoints[i-1];
        const POINT& currPt = m_trajectoryPoints[i];
        
        // Calculate distance between points
        int dx = currPt.x - prevPt.x;
        int dy = currPt.y - prevPt.y;
        int dist = static_cast<int>(std::sqrt(dx*dx + dy*dy));
        
        // Add intermediate points if the distance is large enough
        // This helps smooth the trajectory by adding more points where movement is significant
        if (dist > 5) {  // Threshold for adding intermediate points
            // Add intermediate points between prevPt and currPt
            for (int j = 1; j < 5; ++j) {  // Add up to 4 intermediate points
                float t = static_cast<float>(j) / 5.0f;  // Proportion from prevPt to currPt
                POINT intermediatePt;
                intermediatePt.x = static_cast<LONG>(prevPt.x + t * dx);
                intermediatePt.y = static_cast<LONG>(prevPt.y + t * dy);
                m_smoothedPoints.push_back(intermediatePt);
            }
        }
        
        // Add the current point
        m_smoothedPoints.push_back(currPt);
    }
}

void OverlayUI::updatePosition(int x, int y) {
    if (m_hWnd == nullptr) {
        return;
    }

    // Add new point to trajectory
    POINT newPoint = {x, y};
    m_trajectoryPoints.push_back(newPoint);
    trace("point added...[%d,%d]", x, y);
    
    // Apply smoothing before rendering
    smoothTrajectory();
    
    // Render the updated trajectory
    render(m_smoothedPoints);
}

void OverlayUI::render(const std::vector<POINT>& points) {
    if (m_hWnd == nullptr) {
        return;
    }
    
    if (m_useDirect2D) {
        m_direct2dRenderer.render(points);
    } else {
        m_gdiRenderer.render(points);
    }
}

}  // namespace VirtualDesktop