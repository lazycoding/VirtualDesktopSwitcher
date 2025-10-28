#include <Windows.h>
#include "app.h"

int WINAPI
WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    VirtualDesktop::Application app(hInstance);
    if (!app.initialize()) {
        return -1;
    }
    app.run();
    return 0;
}