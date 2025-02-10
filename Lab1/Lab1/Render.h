#ifndef RENDER_H
#define RENDER_H

#include <dxgi.h>
#include <d3d11.h>

class Render
{
public:
    Render(HWND hWnd) : m_windowHandle(hWnd), m_device(nullptr),
                                m_deviceContext(nullptr),
                                m_swapChain(nullptr),
                                m_renderTargetView(nullptr) {}

    HRESULT Initialize();
    void Shutdown();
    void StartFrame();
    void ResizeViewport();

    void UpdateMousePosition(int x, int y) { m_mousePosition.x = x; m_mousePosition.y = y; }
    HWND GetWindowHandle() const { return m_windowHandle; }

private:
    POINT m_mousePosition = { 0, 0 };
    HRESULT SetupBackBuffer();

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
    HWND m_windowHandle;
};

#endif // RENDER_H