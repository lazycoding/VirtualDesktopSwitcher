#include "DesktopManager.h"
#include <ShlObj.h>
#include <algorithm>
#include <comdef.h>


// 虚拟桌面COM接口定义
MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown {
public:
  virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
      HWND topLevelWindow, BOOL * onCurrentDesktop) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(HWND topLevelWindow,
                                                       GUID * desktopId) = 0;
  virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(HWND topLevelWindow,
                                                        REFGUID desktopId) = 0;
};

MIDL_INTERFACE("af8da486-95bb-4460-b3b7-6e7a6b2962b5")
IVirtualDesktopManagerInternal : public IUnknown {
public:
  virtual HRESULT STDMETHODCALLTYPE GetDesktops(IObjectArray * *desktops) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(GUID * desktopId) = 0;
  virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(GUID * desktopId) = 0;
};

bool DesktopManager::Initialize() {
  HRESULT hr = CoCreateInstance(CLSID_VirtualDesktopManager, nullptr,
                                CLSCTX_ALL, IID_PPV_ARGS(&pDesktopManager));
  if (FAILED(hr))
    return false;

  // 获取内部接口(Windows 10/11实现不同)
  IUnknown *pUnknown = nullptr;
  hr = CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                        IID_PPV_ARGS(&pUnknown));
  if (SUCCEEDED(hr)) {
    pUnknown->QueryInterface(IID_PPV_ARGS(&pDesktopManagerInternal));
    pUnknown->Release();
  }

  return pDesktopManagerInternal != nullptr;
}

void DesktopManager::Shutdown() {
  if (pDesktopManager) {
    pDesktopManager->Release();
    pDesktopManager = nullptr;
  }
  if (pDesktopManagerInternal) {
    pDesktopManagerInternal->Release();
    pDesktopManagerInternal = nullptr;
  }
}

std::vector<DesktopManager::DesktopInfo> DesktopManager::GetDesktops() const {
  std::vector<DesktopInfo> desktops;
  if (!pDesktopManagerInternal)
    return desktops;

  IObjectArray *pObjectArray = nullptr;
  if (SUCCEEDED(pDesktopManagerInternal->GetDesktops(&pObjectArray))) {
    UINT count = 0;
    pObjectArray->GetCount(&count);

    GUID currentDesktopId;
    pDesktopManagerInternal->GetCurrentDesktop(&currentDesktopId);

    for (UINT i = 0; i < count; ++i) {
      IUnknown *pUnknown = nullptr;
      if (SUCCEEDED(pObjectArray->GetAt(i, IID_PPV_ARGS(&pUnknown)))) {
        DesktopInfo info;
        info.index = i;
        info.isCurrent = false;

        // 比较GUID判断当前桌面
        GUID desktopId;
        if (SUCCEEDED(
                pUnknown->QueryInterface(IID_PPV_ARGS(&pDesktopManager)))) {
          pDesktopManager->GetWindowDesktopId(nullptr, &desktopId);
          info.isCurrent = IsEqualGUID(desktopId, currentDesktopId);
        }

        desktops.push_back(info);
        pUnknown->Release();
      }
    }
    pObjectArray->Release();
  }
  return desktops;
}

size_t DesktopManager::GetCurrentDesktopIndex() const {
  auto desktops = GetDesktops();
  auto it =
      std::find_if(desktops.begin(), desktops.end(),
                   [](const DesktopInfo &info) { return info.isCurrent; });
  return it != desktops.end() ? it->index : 0;
}

bool DesktopManager::SwitchToDesktop(size_t index) {
  if (!pDesktopManagerInternal)
    return false;

  auto desktops = GetDesktops();
  if (index >= desktops.size())
    return false;

  IObjectArray *pObjectArray = nullptr;
  if (SUCCEEDED(pDesktopManagerInternal->GetDesktops(&pObjectArray))) {
    IUnknown *pUnknown = nullptr;
    if (SUCCEEDED(pObjectArray->GetAt(index, IID_PPV_ARGS(&pUnknown)))) {
      GUID desktopId;
      if (SUCCEEDED(pUnknown->QueryInterface(IID_PPV_ARGS(&pDesktopManager)))) {
        pDesktopManager->GetWindowDesktopId(nullptr, &desktopId);
        HRESULT hr = pDesktopManagerInternal->SwitchDesktop(&desktopId);
        pUnknown->Release();
        return SUCCEEDED(hr);
      }
      pUnknown->Release();
    }
    pObjectArray->Release();
  }
  return false;
}

bool DesktopManager::MoveWindowToDesktop(HWND hWnd, size_t index) {
  if (!pDesktopManager || !pDesktopManagerInternal)
    return false;

  auto desktops = GetDesktops();
  if (index >= desktops.size())
    return false;

  IObjectArray *pObjectArray = nullptr;
  if (SUCCEEDED(pDesktopManagerInternal->GetDesktops(&pObjectArray))) {
    IUnknown *pUnknown = nullptr;
    if (SUCCEEDED(pObjectArray->GetAt(index, IID_PPV_ARGS(&pUnknown)))) {
      GUID desktopId;
      if (SUCCEEDED(pUnknown->QueryInterface(IID_PPV_ARGS(&pDesktopManager)))) {
        pDesktopManager->GetWindowDesktopId(nullptr, &desktopId);
        HRESULT hr = pDesktopManager->MoveWindowToDesktop(hWnd, desktopId);
        pUnknown->Release();
        return SUCCEEDED(hr);
      }
      pUnknown->Release();
    }
    pObjectArray->Release();
  }
  return false;
}
