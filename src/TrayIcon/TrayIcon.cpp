#include "TrayIcon.h"
#include <shellapi.h>
#include "resource.h"
#include <windows.h>

namespace VirtualDesktop {

    TrayIcon::TrayIcon(HINSTANCE hInstance, const std::wstring& tooltip)
        : m_hInstance(hInstance), m_tooltip(tooltip) {
    }

    TrayIcon::~TrayIcon() { shutdown(); }

    bool TrayIcon::initialize() {
        const wchar_t* className = L"VirtualDesktopSwitcherTrayIconWindow";

        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = m_hInstance;
        wc.lpszClassName = className;

        if (!RegisterClassW(&wc)) {
            return false;
        }

        // Pass 'this' as lpParam so WM_CREATE handler can store the pointer in GWLP_USERDATA
        m_hwnd = CreateWindowW(className, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr,
            m_hInstance, this);
        if (!m_hwnd) {
            return false;
        }

        m_notifyIconData.cbSize = sizeof(NOTIFYICONDATAW);
        m_notifyIconData.hWnd = m_hwnd;
        m_notifyIconData.uID = 1;
        m_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        m_notifyIconData.uCallbackMessage = WM_APP + 1;
        wcsncpy_s(m_notifyIconData.szTip,
            sizeof(m_notifyIconData.szTip) / sizeof(wchar_t), m_tooltip.c_str(),
            _TRUNCATE);

        // Robust icon loading: ensure we have a valid module handle, try LoadImageW
        HINSTANCE hInst = m_hInstance ? m_hInstance : GetModuleHandleW(NULL);

        HICON hIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if (!hIcon) {
            // As a last resort use the standard application icon
            hIcon = LoadIconW(NULL, IDI_APPLICATION);
        }
        m_notifyIconData.hIcon = hIcon;

        return Shell_NotifyIconW(NIM_ADD, &m_notifyIconData);
    }

    void TrayIcon::shutdown() {
        if (m_hwnd) {
            Shell_NotifyIconW(NIM_DELETE, &m_notifyIconData);
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    void TrayIcon::addMenuItem(const std::wstring& label, MenuCallback callback) {
        m_menuItems.emplace_back(label, callback);
    }

    void TrayIcon::showNotification(const std::wstring& title,
        const std::wstring& message) {
        m_notifyIconData.uFlags = NIF_INFO;
        wcsncpy_s(m_notifyIconData.szInfo,
            sizeof(m_notifyIconData.szInfo) / sizeof(wchar_t), message.c_str(),
            _TRUNCATE);
        wcsncpy_s(m_notifyIconData.szInfoTitle,
            sizeof(m_notifyIconData.szInfoTitle) / sizeof(wchar_t),
            title.c_str(), _TRUNCATE);
        Shell_NotifyIconW(NIM_MODIFY, &m_notifyIconData);
    }

    LRESULT CALLBACK TrayIcon::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
        LPARAM lParam) {
        if (uMsg == WM_CREATE) {
            // Store the pointer passed via CreateWindow lpParam into GWLP_USERDATA
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            if (cs && cs->lpCreateParams) {
                SetWindowLongPtr(hwnd, GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            }
        }
        else {
            auto* pThis =
                reinterpret_cast<TrayIcon*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (uMsg == WM_APP + 1 && lParam == WM_RBUTTONUP) {
                if (pThis) {
                    pThis->createMenu();
                }
            }
            else if (uMsg == WM_COMMAND) {
                if (pThis) {
                    int id = LOWORD(wParam);
                    pThis->onCommand(id);
                }
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void TrayIcon::onCommand(int id)
    {
        if (id >= 1) {
            size_t idx = static_cast<size_t>(id - 1);
            if (idx < m_menuItems.size()) {
                auto& cb = m_menuItems[idx].second;
                if (cb) {
                    cb();
                }
            }
        }
    }

    void TrayIcon::createMenu() {
        HMENU hMenu = CreatePopupMenu();
        for (size_t i = 0; i < m_menuItems.size(); ++i) {
            AppendMenuW(hMenu, MF_STRING, i + 1, m_menuItems[i].first.c_str());
        }

        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(m_hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
        PostMessage(m_hwnd, WM_NULL, 0, 0);
        DestroyMenu(hMenu);
    }

} // namespace VirtualDesktop} // namespace VirtualDesktop