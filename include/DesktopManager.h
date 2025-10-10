#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <shobjidl.h>

// 使用Windows SDK中已定义的IVirtualDesktopManager接口
// 自定义内部接口定义
struct IVirtualDesktopManagerInternal : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetDesktops(IObjectArray **ppDesktops) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(GUID *pDesktopId) = 0;
    virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(GUID *pDesktopId) = 0;
};

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
  ::IVirtualDesktopManager *pDesktopManager = nullptr;
  IVirtualDesktopManagerInternal *pDesktopManagerInternal = nullptr;
};