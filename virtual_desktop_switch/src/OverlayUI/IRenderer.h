#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace VirtualDesktop {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void setTrailStyle(const std::string& colorHex, float lineWidth) = 0;
    virtual bool initialize(HWND hwndParent) = 0;
    virtual void resizeForMonitors() = 0;
    virtual void render(const std::vector<POINT>& points) = 0;
    virtual void clear() = 0;
};

}  // namespace VirtualDesktop
