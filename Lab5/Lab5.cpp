#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <windows.h>
#include <DirectXMath.h>
#include "DDSTextureLoader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

#define MAX_STRING_LENGTH 100
WCHAR windowTitle[MAX_STRING_LENGTH] = L"3D Graphics App by Khamidullin Ilsaf";
WCHAR windowClassName[MAX_STRING_LENGTH] = L"GraphicsAppClass";

ID3D11Device* graphicsDevice = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTarget = nullptr;
ID3D11DepthStencilView* depthBuffer = nullptr;

ID3D11VertexShader* mainVertexShader = nullptr;
ID3D11PixelShader* mainPixelShader = nullptr;
ID3D11InputLayout* vertexInputLayout = nullptr;
ID3D11Buffer* vertexDataBuffer = nullptr;
ID3D11Buffer* modelTransformBuffer = nullptr;
ID3D11Buffer* viewProjectionBuffer = nullptr;
ID3D11ShaderResourceView* cubeTexture = nullptr;
ID3D11SamplerState* textureSampler = nullptr;

ID3D11VertexShader* skyVertexShader = nullptr;
ID3D11PixelShader* skyPixelShader = nullptr;
ID3D11InputLayout* skyInputLayout = nullptr;
ID3D11Buffer* skyVertexBuffer = nullptr;
ID3D11Buffer* skyViewProjBuffer = nullptr;
ID3D11ShaderResourceView* skyTexture = nullptr;

ID3D11PixelShader* alphaPixelShader = nullptr;
ID3D11Buffer* alphaBuffer = nullptr;

ID3D11BlendState* blendState = nullptr;
ID3D11DepthStencilState* depthStateTransparent = nullptr;

float cubeRotation = 0.0f;
float cameraRotation = 0.0f;
bool isMouseDragging = false;
POINT lastMousePosition = { 0, 0 };
float cameraHorizontalAngle = 0.0f;
float cameraVerticalAngle = 0.0f;

struct VertexData
{
    float posX, posY, posZ;
    float texU, texV;
};

struct SkyVertex
{
    float x, y, z;
};

VertexData cubeVertices[] =
{
    { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f, 1.0f, 0.0f },

    { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
    { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },

    {  0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

    {  0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f },
    {  0.5f,  0.5f, -0.5f, 0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    { -0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
    { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },

    {  0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

    {  0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f },
    {  0.5f,  0.5f,  0.5f, 0.0f, 0.0f },

    { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f, 1.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

    { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f, 1.0f, 0.0f },
    { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f, 1.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f, 1.0f, 0.0f },
    { -0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
};

SkyVertex skyboxVertices[] =
{
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f },

    {  1.0f, -1.0f,  1.0f },
    {  1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    {  1.0f, -1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    { -1.0f, -1.0f,  1.0f },

    { -1.0f, -1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    { -1.0f,  1.0f, -1.0f },
    { -1.0f, -1.0f,  1.0f },
    { -1.0f,  1.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f },

    { 1.0f, -1.0f, -1.0f },
    { 1.0f,  1.0f, -1.0f },
    { 1.0f,  1.0f,  1.0f },
    { 1.0f, -1.0f, -1.0f },
    { 1.0f,  1.0f,  1.0f },
    { 1.0f, -1.0f,  1.0f },

    { -1.0f, 1.0f, -1.0f },
    { -1.0f, 1.0f,  1.0f },
    { 1.0f, 1.0f,  1.0f },
    { -1.0f, 1.0f, -1.0f },
    { 1.0f, 1.0f,  1.0f },
    { 1.0f, 1.0f, -1.0f },

    { -1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f, -1.0f },
    { 1.0f, -1.0f, -1.0f },
    { -1.0f, -1.0f,  1.0f },
    { 1.0f, -1.0f, -1.0f },
    { 1.0f, -1.0f,  1.0f },
};

ATOM RegisterWindowClass(HINSTANCE hInstance);
BOOL InitializeWindow(HINSTANCE, int);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
HRESULT InitializeDirect3D(HWND hWnd);
HRESULT InitializeResources();
void ReleaseResources();
void RenderFrame();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    RegisterWindowClass(hInstance);
    if (!InitializeWindow(hInstance, nCmdShow))
        return FALSE;

    HWND hWnd = FindWindow(windowClassName, windowTitle);
    if (!hWnd)
        return FALSE;

    if (FAILED(InitializeDirect3D(hWnd)))
        return FALSE;

    if (FAILED(InitializeResources()))
    {
        ReleaseResources();
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
    ReleaseResources();
    return (int)msg.wParam;
}

ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProcedure;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = windowClassName;
    return RegisterClassExW(&wcex);
}

BOOL InitializeWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(windowClassName, windowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600,
        nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

HRESULT InitializeDirect3D(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    UINT creationFlags = 0;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        creationFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &swapDesc, &swapChain,
        &graphicsDevice, &featureLevel, &deviceContext);
    if (FAILED(result))
        return result;

    ID3D11Texture2D* backBuffer = nullptr;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result))
        return result;
    result = graphicsDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    backBuffer->Release();
    if (FAILED(result))
        return result;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* depthTexture = nullptr;
    result = graphicsDevice->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
    if (FAILED(result))
        return result;
    result = graphicsDevice->CreateDepthStencilView(depthTexture, nullptr, &depthBuffer);
    depthTexture->Release();
    if (FAILED(result))
        return result;

    deviceContext->OMSetRenderTargets(1, &renderTarget, depthBuffer);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<FLOAT>(width);
    viewport.Height = static_cast<FLOAT>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    deviceContext->RSSetViewports(1, &viewport);

    return S_OK;
}

HRESULT InitializeResources()
{
    HRESULT hr = S_OK;
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Vertex Shader Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainVertexShader);
    if (FAILED(hr))
    {
        shaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = graphicsDevice->CreateInputLayout(layout, 2, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &vertexInputLayout);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Pixel Shader Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VertexData) * ARRAYSIZE(cubeVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = cubeVertices;
    hr = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &vertexDataBuffer);
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(XMMATRIX);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&bufferDesc, nullptr, &modelTransformBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC vpDesc = {};
    vpDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpDesc.ByteWidth = sizeof(XMMATRIX);
    vpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = graphicsDevice->CreateBuffer(&vpDesc, nullptr, &viewProjectionBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(graphicsDevice, L"cube.dds", nullptr, &cubeTexture);
    if (FAILED(hr))
        return hr;
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = graphicsDevice->CreateSamplerState(&samplerDesc, &textureSampler);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Skybox_VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Skybox VS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &skyVertexShader);
    if (FAILED(hr))
    {
        shaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC skyLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = graphicsDevice->CreateInputLayout(skyLayout, 1, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &skyInputLayout);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Skybox_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Skybox PS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &skyPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(SkyVertex) * ARRAYSIZE(skyboxVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyboxVertices;
    hr = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &skyVertexBuffer);
    if (FAILED(hr))
        return hr;

    vpDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpDesc.ByteWidth = sizeof(XMMATRIX);
    vpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = graphicsDevice->CreateBuffer(&vpDesc, nullptr, &skyViewProjBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(graphicsDevice, L"skybox.dds", nullptr, &skyTexture);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Transparent_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Transparency PS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &alphaPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC alphaDesc = {};
    alphaDesc.Usage = D3D11_USAGE_DEFAULT;
    alphaDesc.ByteWidth = sizeof(XMFLOAT4);
    alphaDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    alphaDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&alphaDesc, nullptr, &alphaBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = graphicsDevice->CreateBlendState(&blendStateDesc, &blendState);
    if (FAILED(hr))
        return hr;

    D3D11_DEPTH_STENCIL_DESC depthDescTrans = {};
    depthDescTrans.DepthEnable = true;
    depthDescTrans.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDescTrans.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = graphicsDevice->CreateDepthStencilState(&depthDescTrans, &depthStateTransparent);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void ReleaseResources()
{
    if (deviceContext)
        deviceContext->ClearState();

    if (vertexDataBuffer)        vertexDataBuffer->Release();
    if (vertexInputLayout)       vertexInputLayout->Release();
    if (mainVertexShader)        mainVertexShader->Release();
    if (mainPixelShader)         mainPixelShader->Release();
    if (modelTransformBuffer)    modelTransformBuffer->Release();
    if (viewProjectionBuffer)    viewProjectionBuffer->Release();
    if (cubeTexture)             cubeTexture->Release();
    if (textureSampler)          textureSampler->Release();
    if (renderTarget)            renderTarget->Release();
    if (depthBuffer)             depthBuffer->Release();
    if (swapChain)               swapChain->Release();
    if (deviceContext)           deviceContext->Release();
    if (graphicsDevice)          graphicsDevice->Release();

    if (skyVertexBuffer)         skyVertexBuffer->Release();
    if (skyInputLayout)          skyInputLayout->Release();
    if (skyVertexShader)         skyVertexShader->Release();
    if (skyPixelShader)          skyPixelShader->Release();
    if (skyViewProjBuffer)       skyViewProjBuffer->Release();
    if (skyTexture)              skyTexture->Release();

    if (alphaPixelShader)        alphaPixelShader->Release();
    if (alphaBuffer)             alphaBuffer->Release();
    if (blendState)              blendState->Release();
    if (depthStateTransparent)   depthStateTransparent->Release();
}

void RenderFrame()
{
    deviceContext->OMSetRenderTargets(1, &renderTarget, depthBuffer);

    float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTarget, clearColor);
    deviceContext->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

    RECT clientRect;
    GetClientRect(FindWindow(windowClassName, windowTitle), &clientRect);
    float aspectRatio = static_cast<float>(clientRect.right - clientRect.left) / (clientRect.bottom - clientRect.top);
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

    float cameraRadius = 8.0f;
    float cameraX = cameraRadius * sinf(cameraHorizontalAngle) * cosf(cameraVerticalAngle);
    float cameraY = cameraRadius * sinf(cameraVerticalAngle);
    float cameraZ = cameraRadius * cosf(cameraHorizontalAngle) * cosf(cameraVerticalAngle);
    XMVECTOR eyePosition = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    XMVECTOR lookAtPoint = XMVectorZero();
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, lookAtPoint, upDirection);

    XMMATRIX skyView = viewMatrix;
    skyView.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX skyViewProj = XMMatrixTranspose(skyView * projection);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    if (SUCCEEDED(deviceContext->Map(skyViewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &skyViewProj, sizeof(XMMATRIX));
        deviceContext->Unmap(skyViewProjBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* skyDepthState = nullptr;
    graphicsDevice->CreateDepthStencilState(&depthDesc, &skyDepthState);
    deviceContext->OMSetDepthStencilState(skyDepthState, 0);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_FRONT;
    rasterDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* skyRasterState = nullptr;
    if (SUCCEEDED(graphicsDevice->CreateRasterizerState(&rasterDesc, &skyRasterState)))
    {
        deviceContext->RSSetState(skyRasterState);
    }

    UINT vertexStride = sizeof(SkyVertex);
    UINT vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &skyVertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(skyInputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(skyVertexShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &skyViewProjBuffer);
    deviceContext->PSSetShader(skyPixelShader, nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, &skyTexture);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);
    deviceContext->Draw(ARRAYSIZE(skyboxVertices), 0);

    skyDepthState->Release();
    if (skyRasterState)
    {
        skyRasterState->Release();
        deviceContext->RSSetState(nullptr);
    }

    deviceContext->OMSetDepthStencilState(nullptr, 0);

    cubeRotation += 0.01f;
    XMMATRIX modelMatrix = XMMatrixRotationY(cubeRotation);
    XMMATRIX modelMatrixTransposed = XMMatrixTranspose(modelMatrix);
    deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &modelMatrixTransposed, 0, 0);

    XMMATRIX cubeViewProj = XMMatrixTranspose(viewMatrix * projection);
    if (SUCCEEDED(deviceContext->Map(viewProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &cubeViewProj, sizeof(XMMATRIX));
        deviceContext->Unmap(viewProjectionBuffer, 0);
    }

    vertexStride = sizeof(VertexData);
    vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexDataBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(vertexInputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(mainVertexShader, nullptr, 0);
    deviceContext->PSSetShader(mainPixelShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &modelTransformBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &viewProjectionBuffer);
    deviceContext->PSSetShaderResources(0, 1, &cubeTexture);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    static float orbitAngle = 0.0f;
    orbitAngle += 0.005f;
    XMMATRIX orbitModel = XMMatrixRotationY(orbitAngle) * XMMatrixTranslation(3.0f, 0.0f, 0.0f) * XMMatrixRotationY(-orbitAngle);
    XMMATRIX orbitModelTransposed = XMMatrixTranspose(orbitModel);
    deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &orbitModelTransposed, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    float blendFactors[4] = { 0, 0, 0, 0 };
    deviceContext->OMSetBlendState(blendState, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(depthStateTransparent, 0);

    ID3D11ShaderResourceView* nullResources[1] = { nullptr };
    deviceContext->PSSetShaderResources(0, 1, nullResources);

    static float blueAnimation = 0.0f;
    blueAnimation += 0.02f;
    float blueZOffset = 2.0f * sinf(blueAnimation);
    XMMATRIX blueModel = XMMatrixTranslation(-2.0f, 0.0f, blueZOffset);
    XMVECTOR bluePosition = XMVectorSet(-2.0f, 0.0f, blueZOffset, 1.0f);

    static float greenAnimation = 0.0f;
    greenAnimation += 0.02f;
    float greenYOffset = 2.0f * sinf(greenAnimation);
    XMMATRIX greenModel = XMMatrixTranslation(2.0f, greenYOffset, 0.0f);
    XMVECTOR greenPosition = XMVectorSet(2.0f, greenYOffset, 0.0f, 1.0f);

    float blueDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(bluePosition, eyePosition)));
    float greenDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(greenPosition, eyePosition)));

    deviceContext->PSSetShader(alphaPixelShader, nullptr, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &alphaBuffer);

    if (blueDistance >= greenDistance)
    {
        XMMATRIX blueModelTransposed = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &blueModelTransposed, 0, 0);
        XMFLOAT4 blueColor = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(alphaBuffer, 0, nullptr, &blueColor, 0, 0);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX greenModelTransposed = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &greenModelTransposed, 0, 0);
        XMFLOAT4 greenColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(alphaBuffer, 0, nullptr, &greenColor, 0, 0);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }
    else
    {
        XMMATRIX greenModelTransposed = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &greenModelTransposed, 0, 0);
        XMFLOAT4 greenColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(alphaBuffer, 0, nullptr, &greenColor, 0, 0);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX blueModelTransposed = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelTransformBuffer, 0, nullptr, &blueModelTransposed, 0, 0);
        XMFLOAT4 blueColor = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(alphaBuffer, 0, nullptr, &blueColor, 0, 0);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }

    deviceContext->OMSetBlendState(nullptr, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(nullptr, 0);

    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        isMouseDragging = true;
        lastMousePosition.x = LOWORD(lParam);
        lastMousePosition.y = HIWORD(lParam);
        SetCapture(hWnd);
        break;
    case WM_LBUTTONUP:
        isMouseDragging = false;
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        if (isMouseDragging)
        {
            int currentX = LOWORD(lParam);
            int currentY = HIWORD(lParam);
            int deltaX = currentX - lastMousePosition.x;
            int deltaY = currentY - lastMousePosition.y;
            cameraHorizontalAngle += deltaX * 0.005f;
            cameraVerticalAngle += deltaY * 0.005f;
            if (cameraVerticalAngle > XM_PIDIV2 - 0.01f)
                cameraVerticalAngle = XM_PIDIV2 - 0.01f;
            if (cameraVerticalAngle < -XM_PIDIV2 + 0.01f)
                cameraVerticalAngle = -XM_PIDIV2 + 0.01f;
            lastMousePosition.x = currentX;
            lastMousePosition.y = currentY;
        }
        break;
    case WM_KEYDOWN:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
