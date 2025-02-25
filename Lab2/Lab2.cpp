#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define MAX_LOADSTRING 100
WCHAR windowTitle[MAX_LOADSTRING] = L"Khamidullin Ilsab Lab2";
WCHAR windowClass[MAX_LOADSTRING] = L"LAB2";

ID3D11Device* device = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

ID3D11VertexShader* vertexShader = nullptr;
ID3D11PixelShader* pixelShader = nullptr;
ID3D11InputLayout* inputLayout = nullptr;
ID3D11Buffer* vertexBuffer = nullptr;

float currentColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float targetColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

ATOM RegisterWindowClass(HINSTANCE hInstance);
BOOL InitializeInstance(HINSTANCE, int);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitializeDirect3D(HWND hWnd);
HRESULT InitializeGraphics();
void CleanupDirect3D();
void RenderFrame();
void GenerateRandomColor();

struct Vertex
{
    float x, y, z;
    float r, g, b, a;
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    RegisterWindowClass(hInstance);

    if (!InitializeInstance(hInstance, nCmdShow))
        return FALSE;

    HWND hWnd = FindWindow(windowClass, windowTitle);
    if (!hWnd)
        return FALSE;

    if (FAILED(InitializeDirect3D(hWnd)))
        return FALSE;

    if (FAILED(InitializeGraphics()))
    {
        CleanupDirect3D();
        return FALSE;
    }

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            RenderFrame();
        }
    }

    CleanupDirect3D();
    return (int)msg.wParam;
}

ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = windowClass;

    return RegisterClassExW(&wcex);
}

BOOL InitializeInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(windowClass, windowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

HRESULT InitializeDirect3D(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevelArray, 1,
        D3D11_SDK_VERSION, &sd, &swapChain,
        &device, &featureLevel, &deviceContext);
    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr))
        return hr;

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &vp);

    return S_OK;
}

HRESULT InitializeGraphics()
{
    HRESULT hr = S_OK;
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Vertex Shader Compile Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }

    hr = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
        nullptr, &vertexShader);
    if (FAILED(hr))
    {
        vertexShaderBlob->Release();
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layoutDesc);
    hr = device->CreateInputLayout(layoutDesc, numElements, vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(), &inputLayout);
    vertexShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    ID3DBlob* pixelShaderBlob = nullptr;
    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Pixel Shader Compile Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }

    hr = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(),
        nullptr, &pixelShader);
    pixelShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    Vertex vertices[] =
    {
        {  0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f },
    };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertex) * 3;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void CleanupDirect3D()
{
    if (deviceContext)
        deviceContext->ClearState();

    if (vertexBuffer)
        vertexBuffer->Release();
    if (inputLayout)
        inputLayout->Release();
    if (vertexShader)
        vertexShader->Release();
    if (pixelShader)
        pixelShader->Release();

    if (renderTargetView)
        renderTargetView->Release();
    if (swapChain)
        swapChain->Release();
    if (deviceContext)
        deviceContext->Release();
    if (device)
        device->Release();
}

void GenerateRandomColor()
{
    float newColor[3] = { 0.0f, 0.0f, 0.0f };
    do {
        newColor[0] = static_cast<float>(rand()) / RAND_MAX;
        newColor[1] = static_cast<float>(rand()) / RAND_MAX;
        newColor[2] = static_cast<float>(rand()) / RAND_MAX;
    } while ((newColor[0] < 0.05f && newColor[1] < 0.05f && newColor[2] < 0.05f) ||
        (newColor[0] > 0.95f && newColor[1] > 0.95f && newColor[2] > 0.95f));
    targetColor[0] = newColor[0];
    targetColor[1] = newColor[1];
    targetColor[2] = newColor[2];
}

void RenderFrame()
{
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    float lerpFactor = 0.01f;
    for (int i = 0; i < 3; ++i)
        currentColor[i] += (targetColor[i] - currentColor[i]) * lerpFactor;

    if (fabs(currentColor[0] - targetColor[0]) < 0.01f &&
        fabs(currentColor[1] - targetColor[1]) < 0.01f &&
        fabs(currentColor[2] - targetColor[2]) < 0.01f)
    {
        GenerateRandomColor();
    }

    float clearColor[4] = { currentColor[0], currentColor[1], currentColor[2], 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(inputLayout);

    deviceContext->VSSetShader(vertexShader, nullptr, 0);
    deviceContext->PSSetShader(pixelShader, nullptr, 0);

    deviceContext->Draw(3, 0);
    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (swapChain && wParam != SIZE_MINIMIZED)
        {
            if (renderTargetView)
            {
                deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
                renderTargetView->Release();
                renderTargetView = nullptr;
            }
            HRESULT hr = swapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            if (SUCCEEDED(hr))
            {
                ID3D11Texture2D* buffer = nullptr;
                hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&buffer);
                if (SUCCEEDED(hr))
                {
                    hr = device->CreateRenderTargetView(buffer, nullptr, &renderTargetView);
                    buffer->Release();
                }
                if (SUCCEEDED(hr))
                {
                    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    D3D11_VIEWPORT vp = {};
                    vp.Width = static_cast<FLOAT>(rc.right - rc.left);
                    vp.Height = static_cast<FLOAT>(rc.bottom - rc.top);
                    vp.MinDepth = 0.0f;
                    vp.MaxDepth = 1.0f;
                    deviceContext->RSSetViewports(1, &vp);
                }
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}