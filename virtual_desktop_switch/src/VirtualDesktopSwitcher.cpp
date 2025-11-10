#include "VirtualDesktopSwitcher.h"
#include "app.h"

using namespace VirtualDesktop;
void run(VirtualDesktopApp* app) {
    Application application(app->hInstance);
    application.initialize();
    if (app->useSelfMessageLoop) {
        application.run();
    }
}

HINSTANCE g_hInst = nullptr;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH)
        g_hInst = hinstDLL;
    return TRUE;
}
