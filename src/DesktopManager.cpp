#include "../include/DesktopManager.h"
#include <ShlObj.h>
#include <algorithm>
#include <comdef.h>

#include <comdef.h>
#include <shobjidl_core.h>

#pragma comment(lib, "ole32.lib")

// 使用Windows SDK中已定义的接口
using IVirtualDesktopManagerInternal = IUnknown;

bool DesktopManager::Initialize() {
  HRESULT hr = CoCreateInstance(CLSID_VirtualDesktopManager, nullptr,
                                CLSCTX_ALL, IID_PPV_ARGS(&pDesktopManager));
  if (FAILED(hr))
    return false;

  // 获取虚拟桌面管理器内部接口
  static const CLSID CLSID_ImmersiveShell = {
      0xC2F03A33,
      0x21F5,
      0x47FA,
      {0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39}};
  static const IID IID_IVirtualDesktopManagerInternal = {
      0xAF8DA486,
      0x95BB,
      0x4460,
      {0xB3, 0xB7, 0x6E, 0x7A, 0x6B, 0x29, 0x62, 0xB5}};

  IUnknown *pUnknown = nullptr;
  hr = CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                        IID_PPV_ARGS(&pUnknown));
  if (SUCCEEDED(hr)) {
    pUnknown->QueryInterface(
        IID_IVirtualDesktopManagerInternal,
        reinterpret_cast<void **>(&pDesktopManagerInternal));
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
