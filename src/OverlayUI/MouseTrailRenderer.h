// MouseTrailRenderer.h
#pragma once
#include <chrono>
#include <d2d1.h>
#include <vector>
#include <windows.h>

#pragma comment(lib, "d2d1.lib")

class MouseTrailRenderer {
public:
    MouseTrailRenderer();
    ~MouseTrailRenderer();

    // 初始化，参数为父窗口（通常是全屏透明窗口）
    bool Initialize(HWND hwndParent);

    // 当显示器配置变化时重新计算虚拟屏并重建资源
    void ResizeForMonitors();

    // 绘制接口：传入屏幕坐标点数组
    void Render(const std::vector<POINT>& pts);

    // 清除当前绘制内容
    void Clear();

private:
    struct FPoint {
        float x, y;
    };

    // 资源创建销毁
    void CreateDIBAndRenderTarget();
    void DestroyDIBAndRenderTarget();

    // 内部辅助
    static BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM);
    void ComputeVirtualScreenRect();
    FPoint CatmullRom(const FPoint& p0, const FPoint& p1, const FPoint& p2,
        const FPoint& p3, float t);

private:
    HWND m_hwnd = nullptr;
    ID2D1Factory* m_factory = nullptr;
    ID2D1DCRenderTarget* m_rt = nullptr;
    ID2D1SolidColorBrush* m_brush = nullptr;
    ID2D1StrokeStyle* m_stroke = nullptr;

    HDC m_memDC = nullptr;
    HBITMAP m_hDIB = nullptr;
    void* m_pBits = nullptr;

    RECT m_rcVirtual = { 0 };
    int m_width = 0;
    int m_height = 0;

    const float m_lineWidth = 6.0f;
    const int m_steps = 8;
};
