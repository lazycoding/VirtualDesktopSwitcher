#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <vector>

class OverlayUI {
public:
    struct LineSegment {
        D2D1_POINT_2F start;
        D2D1_POINT_2F end;
        float opacity;
    };

    bool Initialize();
    void Shutdown();
    
    void AddTrailPoint(POINT pt);
    void Render();
    void Clear();

    // 样式配置
    void SetLineColor(D2D1::ColorF color);
    void SetLineWidth(float width);
    void SetFadeSpeed(float speed);

private:
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr;
    HWND hWnd = nullptr;
    
    std::vector<LineSegment> trailSegments;
    D2D1::ColorF lineColor = D2D1::ColorF(D2D1::ColorF::Blue);
    float lineWidth = 2.0f;
    float fadeSpeed = 0.1f;
    
    bool CreateDeviceResources();
    void DiscardDeviceResources();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};