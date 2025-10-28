#include "utils.h"
#include <cstdarg>
#include <iostream>

namespace VirtualDesktop {
void Trace(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[1024] = {0};
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    std::cout << buffer << std::endl;
}
}  // namespace VirtualDesktop