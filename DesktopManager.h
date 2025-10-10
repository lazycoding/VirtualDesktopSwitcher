#pragma once
#include <Windows.h>
#include <vector>

class DesktopManager {
public:
    struct DesktopInfo {
        size_t index;
        std::wstring name;
        bool isCurrent;
    };

    bool Initialize();
    void Shutdown();

    std::vector<DesktopInfo> GetDesktops() const;
    size_t GetCurrentDesktopIndex() const;
    bool SwitchToDesktop(size_t index);
    bool MoveWindowToDesktop(HWND hWnd, size_t index);

private:
    IVirtualDesktopManager* pDesktopManager = nullptr;
    IVirtualDesktopManagerInternal* pDesktopManagerInternal = nullptr;
};