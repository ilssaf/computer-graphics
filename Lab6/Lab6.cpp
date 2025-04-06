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

#define MAX_STRING_SIZE 100
WCHAR appTitle[MAX_STRING_SIZE] = L"3D Graphics Application by Khamidullin Ilsaf";
WCHAR appWindowClass[MAX_STRING_SIZE] = L"GraphicsAppWindowClass";

ID3D11Device* graphicsDevice = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;
ID3D11DepthStencilView* depthStencilView = nullptr;
ID3D11VertexShader* transparentVertexShader = nullptr;
ID3D11VertexShader* mainVertexShader = nullptr;
ID3D11PixelShader* mainPixelShader = nullptr;
ID3D11InputLayout* vertexLayout = nullptr;
ID3D11Buffer* vertexBuffer = nullptr;
ID3D11Buffer* modelMatrixBuffer = nullptr;
ID3D11Buffer* viewProjectionBuffer = nullptr;
ID3D11ShaderResourceView* cubeTexture = nullptr;
ID3D11ShaderResourceView* cubeNormalTexture = nullptr;
ID3D11SamplerState* linearSampler = nullptr;

ID3D11VertexShader* skyboxVertexShader = nullptr;
ID3D11PixelShader* skyboxPixelShader = nullptr;
ID3D11InputLayout* skyboxInputLayout = nullptr;
ID3D11Buffer* skyboxVertexBuffer = nullptr;
ID3D11Buffer* skyboxViewProjBuffer = nullptr;
ID3D11ShaderResourceView* skyboxTexture = nullptr;

ID3D11PixelShader* transparentPixelShader = nullptr;
ID3D11Buffer* transparentColorBuffer = nullptr;
ID3D11Buffer* lightDataBuffer = nullptr;
ID3D11PixelShader* lightPixelShader = nullptr;
ID3D11Buffer* lightColorBuffer = nullptr;
ID3D11Buffer* blueColorBuffer = nullptr;
ID3D11Buffer* greenColorBuffer = nullptr;

ID3D11BlendState* alphaBlendState = nullptr;
ID3D11DepthStencilState* transparentDepthState = nullptr;

float cubeRotationAngle = 0.0f;
float cameraRotationAngle = 0.0f;
bool isMouseDragging = false;
POINT lastMousePosition = { 0, 0 };
float cameraAzimuth = 0.0f;
float cameraElevation = 0.0f;

struct VertexData
{
    float posX, posY, posZ;
    float normalX, normalY, normalZ;
    float texU, texV;
};

VertexData cubeVertices[] =
{
    { -0.5f, -0.5f,  0.5f,    0,0,1,   0.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f,    0,0,1,   1.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,    0,0,1,   1.0f, 0.0f },
    { -0.5f, -0.5f,  0.5f,    0,0,1,   0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,    0,0,1,   1.0f, 0.0f },
    { -0.5f,  0.5f,  0.5f,    0,0,1,   0.0f, 0.0f },

    {  0.5f, -0.5f, -0.5f,    0,0,-1,  0.0f, 1.0f },
    { -0.5f, -0.5f, -0.5f,    0,0,-1,  1.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f,    0,0,-1,  1.0f, 0.0f },
    {  0.5f, -0.5f, -0.5f,    0,0,-1,  0.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f,    0,0,-1,  1.0f, 0.0f },
    {  0.5f,  0.5f, -0.5f,    0,0,-1,  0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f,   -1,0,0,   0.0f, 1.0f },
    { -0.5f, -0.5f,  0.5f,   -1,0,0,   1.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f,   -1,0,0,   1.0f, 0.0f },
    { -0.5f, -0.5f, -0.5f,   -1,0,0,   0.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f,   -1,0,0,   1.0f, 0.0f },
    { -0.5f,  0.5f, -0.5f,   -1,0,0,   0.0f, 0.0f },

    {  0.5f, -0.5f,  0.5f,    1,0,0,   0.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f,    1,0,0,   1.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,    1,0,0,   1.0f, 0.0f },
    {  0.5f, -0.5f,  0.5f,    1,0,0,   0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,    1,0,0,   1.0f, 0.0f },
    {  0.5f,  0.5f,  0.5f,    1,0,0,   0.0f, 0.0f },

    { -0.5f,  0.5f,  0.5f,    0,1,0,   0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,    0,1,0,   1.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,    0,1,0,   1.0f, 0.0f },
    { -0.5f,  0.5f,  0.5f,    0,1,0,   0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,    0,1,0,   1.0f, 0.0f },
    { -0.5f,  0.5f, -0.5f,    0,1,0,   0.0f, 0.0f },

    { -0.5f, -0.5f, -0.5f,    0,-1,0,  0.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f,    0,-1,0,  1.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f,    0,-1,0,  1.0f, 0.0f },
    { -0.5f, -0.5f, -0.5f,    0,-1,0,  0.0f, 1.0f },
    {  0.5f, -0.5f,  0.5f,    0,-1,0,  1.0f, 0.0f },
    { -0.5f, -0.5f,  0.5f,    0,-1,0,  0.0f, 0.0f },
};

struct SkyboxVertex
{
    float x, y, z;
};

SkyboxVertex skyboxVertices[] =
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

ATOM RegisterAppWindowClass(HINSTANCE hInstance);
BOOL InitializeAppWindow(HINSTANCE, int);
LRESULT CALLBACK WindowMessageHandler(HWND, UINT, WPARAM, LPARAM);
HRESULT InitializeDirect3D(HWND hWnd);
HRESULT InitializeGraphicsResources();
void CleanupDirect3D();
void RenderScene();

struct LightData
{
    XMFLOAT3 light0Position;
    float padding0;
    XMFLOAT3 light0Color;
    float padding1;
    XMFLOAT3 light1Position;
    float padding2;
    XMFLOAT3 light1Color;
    float padding3;
    XMFLOAT3 ambientColor;
    float padding4;
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    RegisterAppWindowClass(hInstance);
    if (!InitializeAppWindow(hInstance, nCmdShow))
        return FALSE;

    HWND hWnd = FindWindow(appWindowClass, appTitle);
    if (!hWnd)
        return FALSE;

    if (FAILED(InitializeDirect3D(hWnd)))
        return FALSE;

    if (FAILED(InitializeGraphicsResources()))
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
            RenderScene();
        }
    }
    CleanupDirect3D();
    return (int)msg.wParam;
}

ATOM RegisterAppWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowMessageHandler;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = appWindowClass;
    return RegisterClassExW(&wcex);
}

BOOL InitializeAppWindow(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(appWindowClass, appTitle, WS_OVERLAPPEDWINDOW,
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
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        creationFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &swapDesc, &swapChain,
        &graphicsDevice, &featureLevel, &deviceContext);
    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;
    hr = graphicsDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* depthTexture = nullptr;
    hr = graphicsDevice->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
    if (FAILED(hr))
        return hr;
    hr = graphicsDevice->CreateDepthStencilView(depthTexture, nullptr, &depthStencilView);
    depthTexture->Release();
    if (FAILED(hr))
        return hr;

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

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

HRESULT InitializeGraphicsResources()
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
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = graphicsDevice->CreateInputLayout(layout, 3, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &vertexLayout);
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
    hr = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(XMMATRIX);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&bufferDesc, nullptr, &modelMatrixBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC vpBufferDesc = {};
    vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
    vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = graphicsDevice->CreateBuffer(&vpBufferDesc, nullptr, &viewProjectionBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(graphicsDevice, L"cube.dds", nullptr, &cubeTexture);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(graphicsDevice, L"cube_normal.dds", nullptr, &cubeNormalTexture);
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
    hr = graphicsDevice->CreateSamplerState(&samplerDesc, &linearSampler);
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
        nullptr, &skyboxVertexShader);
    if (FAILED(hr))
    {
        shaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = graphicsDevice->CreateInputLayout(skyboxLayout, 1, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &skyboxInputLayout);
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
        nullptr, &skyboxPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(SkyboxVertex) * ARRAYSIZE(skyboxVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyboxVertices;
    hr = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &skyboxVertexBuffer);
    if (FAILED(hr))
        return hr;

    vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
    vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = graphicsDevice->CreateBuffer(&vpBufferDesc, nullptr, &skyboxViewProjBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(graphicsDevice, L"skybox.dds", nullptr, &skyboxTexture);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Transparent_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Transparent PS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &transparentPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC colorBufferDesc = {};
    colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    colorBufferDesc.ByteWidth = sizeof(XMFLOAT4);
    colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colorBufferDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&colorBufferDesc, nullptr, &transparentColorBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufferDesc.ByteWidth = sizeof(LightData);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&lightBufferDesc, nullptr, &lightDataBuffer);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Light_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Light PS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &lightPixelShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightColorDesc = {};
    lightColorDesc.Usage = D3D11_USAGE_DEFAULT;
    lightColorDesc.ByteWidth = sizeof(XMFLOAT4);
    lightColorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightColorDesc.CPUAccessFlags = 0;
    hr = graphicsDevice->CreateBuffer(&lightColorDesc, nullptr, &lightColorBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC colorDesc = {};
    colorDesc.Usage = D3D11_USAGE_DEFAULT;
    colorDesc.ByteWidth = sizeof(XMFLOAT4);
    colorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colorDesc.CPUAccessFlags = 0;

    hr = graphicsDevice->CreateBuffer(&colorDesc, nullptr, &blueColorBuffer);
    if (FAILED(hr)) return hr;

    hr = graphicsDevice->CreateBuffer(&colorDesc, nullptr, &greenColorBuffer);
    if (FAILED(hr)) return hr;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = graphicsDevice->CreateBlendState(&blendDesc, &alphaBlendState);
    if (FAILED(hr))
        return hr;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = graphicsDevice->CreateDepthStencilState(&depthStencilDesc, &transparentDepthState);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Transparent_VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Transparent VS Error", MB_OK);
            errorBlob->Release();
        }
        return hr;
    }
    hr = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &transparentVertexShader);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    return S_OK;
}
void ReleaseResources()
{
    if (deviceContext)
        deviceContext->ClearState();

    if (vertexBuffer)           vertexBuffer->Release();
    if (vertexLayout)           vertexLayout->Release();
    if (mainVertexShader)       mainVertexShader->Release();
    if (mainPixelShader)        mainPixelShader->Release();
    if (modelMatrixBuffer)      modelMatrixBuffer->Release();
    if (viewProjectionBuffer)   viewProjectionBuffer->Release();
    if (cubeTexture)            cubeTexture->Release();
    if (cubeNormalTexture)      cubeNormalTexture->Release();
    if (linearSampler)          linearSampler->Release();
    if (renderTargetView)       renderTargetView->Release();
    if (depthStencilView)       depthStencilView->Release();
    if (swapChain)              swapChain->Release();
    if (deviceContext)          deviceContext->Release();
    if (graphicsDevice)         graphicsDevice->Release();

    if (skyboxVertexBuffer)     skyboxVertexBuffer->Release();
    if (skyboxInputLayout)      skyboxInputLayout->Release();
    if (skyboxVertexShader)     skyboxVertexShader->Release();
    if (skyboxPixelShader)      skyboxPixelShader->Release();
    if (skyboxViewProjBuffer)   skyboxViewProjBuffer->Release();
    if (skyboxTexture)          skyboxTexture->Release();

    if (transparentPixelShader) transparentPixelShader->Release();
    if (transparentColorBuffer) transparentColorBuffer->Release();
    if (alphaBlendState)        alphaBlendState->Release();
    if (transparentDepthState)  transparentDepthState->Release();

    if (lightDataBuffer)        lightDataBuffer->Release();
    if (lightPixelShader)       lightPixelShader->Release();
    if (lightColorBuffer)       lightColorBuffer->Release();
    if (blueColorBuffer)        blueColorBuffer->Release();
    if (greenColorBuffer)       greenColorBuffer->Release();
    if (transparentVertexShader) transparentVertexShader->Release();
}

void RenderScene()
{
    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    RECT clientRect;
    GetClientRect(FindWindow(appWindowClass, appTitle), &clientRect);
    float aspectRatio = static_cast<float>(clientRect.right - clientRect.left) / (clientRect.bottom - clientRect.top);
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

    float cameraRadius = 6.0f;
    float cameraX = cameraRadius * sinf(cameraAzimuth) * cosf(cameraElevation);
    float cameraY = cameraRadius * sinf(cameraElevation);
    float cameraZ = cameraRadius * cosf(cameraAzimuth) * cosf(cameraElevation);
    XMVECTOR eyePosition = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    XMVECTOR lookAtPoint = XMVectorZero();
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, lookAtPoint, upDirection);

    XMMATRIX skyboxView = viewMatrix;
    skyboxView.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX skyboxViewProj = XMMatrixTranspose(skyboxView * projection);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    if (SUCCEEDED(deviceContext->Map(skyboxViewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &skyboxViewProj, sizeof(XMMATRIX));
        deviceContext->Unmap(skyboxViewProjBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* skyboxDepthState = nullptr;
    graphicsDevice->CreateDepthStencilState(&depthDesc, &skyboxDepthState);
    deviceContext->OMSetDepthStencilState(skyboxDepthState, 0);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_FRONT;
    rasterDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* skyboxRasterState = nullptr;
    if (SUCCEEDED(graphicsDevice->CreateRasterizerState(&rasterDesc, &skyboxRasterState)))
    {
        deviceContext->RSSetState(skyboxRasterState);
    }

    UINT vertexStride = sizeof(SkyboxVertex);
    UINT vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &skyboxVertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(skyboxInputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(skyboxVertexShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &skyboxViewProjBuffer);
    deviceContext->PSSetShader(skyboxPixelShader, nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, &skyboxTexture);
    deviceContext->PSSetSamplers(0, 1, &linearSampler);
    deviceContext->Draw(ARRAYSIZE(skyboxVertices), 0);

    skyboxDepthState->Release();
    if (skyboxRasterState)
    {
        skyboxRasterState->Release();
        deviceContext->RSSetState(nullptr);
    }

    deviceContext->OMSetDepthStencilState(nullptr, 0);

    cubeRotationAngle += 0.01f;
    XMMATRIX modelMatrix = XMMatrixRotationY(cubeRotationAngle);
    XMMATRIX modelMatrixTransposed = XMMatrixTranspose(modelMatrix);
    deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &modelMatrixTransposed, 0, 0);

    XMMATRIX cubeViewProj = XMMatrixTranspose(viewMatrix * projection);
    if (SUCCEEDED(deviceContext->Map(viewProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &cubeViewProj, sizeof(XMMATRIX));
        deviceContext->Unmap(viewProjectionBuffer, 0);
    }

    vertexStride = sizeof(VertexData);
    vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(vertexLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(mainVertexShader, nullptr, 0);
    deviceContext->PSSetShader(mainPixelShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &modelMatrixBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &viewProjectionBuffer);
    deviceContext->PSSetConstantBuffers(0, 1, &lightDataBuffer);
    ID3D11ShaderResourceView* cubeTextures[2] = { cubeTexture, cubeNormalTexture };
    deviceContext->PSSetShaderResources(0, 2, cubeTextures);
    deviceContext->PSSetSamplers(0, 1, &linearSampler);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    static float orbitAngle = 0.0f;
    orbitAngle += 0.005f;
    XMMATRIX orbitModel = XMMatrixRotationY(orbitAngle) * XMMatrixTranslation(3.0f, 0.0f, 0.0f) * XMMatrixRotationY(-orbitAngle);
    XMMATRIX orbitModelTransposed = XMMatrixTranspose(orbitModel);
    deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &orbitModelTransposed, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    float blendFactors[4] = { 0, 0, 0, 0 };
    deviceContext->OMSetBlendState(alphaBlendState, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(transparentDepthState, 0);

    ID3D11ShaderResourceView* nullResources[1] = { nullptr };
    deviceContext->PSSetShaderResources(0, 1, &cubeTexture);

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

    deviceContext->PSSetShader(transparentPixelShader, nullptr, 0);
    deviceContext->VSSetShader(transparentVertexShader, nullptr, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &lightDataBuffer);

    if (blueDistance >= greenDistance)
    {
        XMMATRIX blueModelTransposed = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &blueModelTransposed, 0, 0);

        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(blueColorBuffer, 0, nullptr, &blueColor, 0, 0);

        deviceContext->PSSetConstantBuffers(1, 1, &blueColorBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX greenModelTransposed = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &greenModelTransposed, 0, 0);

        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(greenColorBuffer, 0, nullptr, &greenColor, 0, 0);

        deviceContext->PSSetConstantBuffers(1, 1, &greenColorBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }
    else
    {
        XMMATRIX greenModelTransposed = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &greenModelTransposed, 0, 0);

        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(greenColorBuffer, 0, nullptr, &greenColor, 0, 0);

        deviceContext->PSSetConstantBuffers(1, 1, &greenColorBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX blueModelTransposed = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &blueModelTransposed, 0, 0);

        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(blueColorBuffer, 0, nullptr, &blueColor, 0, 0);

        deviceContext->PSSetConstantBuffers(1, 1, &blueColorBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }

    deviceContext->OMSetBlendState(nullptr, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(nullptr, 0);

    LightData lightData;
    lightData.light0Position = XMFLOAT3(1.0f, 0.0f, 0.0f);
    lightData.light0Color = XMFLOAT3(3.0f, 3.0f, 0.0f);
    lightData.light1Position = XMFLOAT3(-1.0f, 0.0f, 0.0f);
    lightData.light1Color = XMFLOAT3(3.0f, 3.0f, 3.0f);
    lightData.ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
    deviceContext->UpdateSubresource(lightDataBuffer, 0, nullptr, &lightData, 0, 0);

    deviceContext->PSSetShader(lightPixelShader, nullptr, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &lightColorBuffer);

    XMMATRIX lightScaleMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);

    XMMATRIX lightTranslate0 = XMMatrixTranslation(1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel0 = XMMatrixTranspose(lightScaleMatrix * lightTranslate0);
    deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &lightModel0, 0, 0);
    XMFLOAT4 lightColor0(1.0f, 1.0f, 0.0f, 1.0f);
    deviceContext->UpdateSubresource(lightColorBuffer, 0, nullptr, &lightColor0, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    XMMATRIX lightTranslate1 = XMMatrixTranslation(-1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel1 = XMMatrixTranspose(lightScaleMatrix * lightTranslate1);
    deviceContext->UpdateSubresource(modelMatrixBuffer, 0, nullptr, &lightModel1, 0, 0);
    XMFLOAT4 lightColor1(1.0f, 1.0f, 1.0f, 1.0f);
    deviceContext->UpdateSubresource(lightColorBuffer, 0, nullptr, &lightColor1, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowMessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
            cameraAzimuth += deltaX * 0.005f;
            cameraElevation += deltaY * 0.005f;
            if (cameraElevation > XM_PIDIV2 - 0.01f)
                cameraElevation = XM_PIDIV2 - 0.01f;
            if (cameraElevation < -XM_PIDIV2 + 0.01f)
                cameraElevation = -XM_PIDIV2 + 0.01f;
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