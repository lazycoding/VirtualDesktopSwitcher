#pragma once
#include <windows.h>
#ifdef BUILDING_DLL
#define VDS_API __declspec(dllexport)
#else
#define VDS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    HINSTANCE hInstance;
} VirtualDesktopApp;

VDS_API void run(VirtualDesktopApp* app);

#ifdef __cplusplus
}
#endif
