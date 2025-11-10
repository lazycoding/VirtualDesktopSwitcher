#include <Windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include "VirtualDesktopSwitcher.h"

void RedirectStdoutToConsole() {
#ifdef _DEBUG
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        std::cout.clear();
        std::cerr.clear();
    }
#endif
}

int WINAPI
WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    RedirectStdoutToConsole();
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "Virtual Desktop Switcher starting with instance..." << hInstance << std::endl;

    VirtualDesktopApp app = {hInstance, true};
    run(&app);

    std::cout << "Application shutting down" << std::endl;
    return 0;
}