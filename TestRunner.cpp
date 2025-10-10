#include "VirtualDesktopSwitcher.h"
#include <iostream>

int main() {
  VirtualDesktopSwitcher app;

  if (!app.Initialize()) {
    std::cerr << "Initialization failed" << std::endl;
    return 1;
  }

  std::cout << "Virtual Desktop Switcher is running..." << std::endl;
  std::cout << "Press Enter to exit..." << std::endl;

  std::cin.get();
  app.Shutdown();
  return 0;
}