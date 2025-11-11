#include "IRenderer.h"
#include "GdiRenderer.h"
#include <memory>

namespace VirtualDesktop {
std::unique_ptr<IRenderer> createRendererByMode(RenderMode mode) {
    switch (mode) {
        case RenderMode::Direct2D:
            // todo: direct2d render
            return nullptr;
        case RenderMode::Gdiplus:
            return std::make_unique<GdiRenderer>();
        default:
            return std::make_unique<GdiRenderer>();
    }
}
}  // namespace VirtualDesktop
