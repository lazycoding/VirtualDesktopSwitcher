#include "IRenderer.h"
#include "DxRenderer.h"
#include "GdiRenderer.h"
#include <memory>

namespace VirtualDesktop {
std::unique_ptr<IRenderer> createRendererByMode(RenderMode mode) {
    // 字符串比较双方忽略大小写
    if (mode == RenderMode::Direct2D) {
        return std::make_unique<DxRenderer>();
    } else if (mode == RenderMode::Gdiplus) {
        return std::make_unique<GdiRenderer>();
    } else {
        return std::make_unique<GdiRenderer>();
    }
}
}  // namespace VirtualDesktop
