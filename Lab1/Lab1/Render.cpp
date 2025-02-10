#include "framework.h"
#include "Render.h"

#include <dxgi.h>
#include <d3d11.h>
#include <math.h>
#define GE_2PI 3.1415926
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

HRESULT Render::Initialize()
{
    HRESULT result;

    IDXGIFactory* pFactory = nullptr;
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    IDXGIAdapter* selectedAdapter = nullptr;
    if (SUCCEEDED(result))
    {
        IDXGIAdapter* adapter = nullptr;
        UINT adapterIdx = 0;
        while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &adapter)))
        {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0)
            {
                selectedAdapter = adapter;
                break;
            }

            adapter->Release();
            adapterIdx++;
        }
    }

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    if (SUCCEEDED(result))
    {
        UINT deviceFlags = 0;
#ifdef _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        result = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                                   deviceFlags, featureLevels, 1, D3D11_SDK_VERSION, 
                                   &m_device, &featureLevel, &m_deviceContext);
    }

    if (SUCCEEDED(result))
    {
        DXGI_SWAP_CHAIN_DESC swapDesc = {};
        swapDesc.BufferCount = 2;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = m_windowHandle;
        swapDesc.SampleDesc.Count = 1;
        swapDesc.Windowed = TRUE;

        result = pFactory->CreateSwapChain(m_device, &swapDesc, &m_swapChain);
    }

    if (SUCCEEDED(result))
    {
        result = SetupBackBuffer();
    }

    if (selectedAdapter)
        selectedAdapter->Release();
    if (pFactory)
        pFactory->Release();

    return result;
}

void Render::Shutdown()
{
    if (m_deviceContext)
    {
        m_deviceContext->ClearState();
        m_deviceContext->Release();
    }
    if (m_renderTargetView)
        m_renderTargetView->Release();
    if (m_swapChain)
        m_swapChain->Release();
    if (m_device)
        m_device->Release();
}

void Render::StartFrame()
{
    RECT rc;
    GetClientRect(m_windowHandle, &rc);
    float normalizedX = static_cast<float>(m_mousePosition.x) / (rc.right - rc.left);
    float normalizedY = static_cast<float>(m_mousePosition.y) / (rc.bottom - rc.top);

    float clearColor[4] = {
        normalizedX,
        normalizedY,
        (normalizedX + normalizedY) / 2,
        1.0f
    };

    m_deviceContext->ClearRenderTargetView(m_renderTargetView, clearColor);
    m_swapChain->Present(0, 0);
}

HRESULT Render::SetupBackBuffer()
{
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;

    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
    backBuffer->Release();
    return hr;
}

void Render::ResizeViewport()
{
    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }

    if (m_swapChain)
    {
        RECT rc;
        GetClientRect(m_windowHandle, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        SetupBackBuffer();

        m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_deviceContext->RSSetViewports(1, &viewport);
    }
}