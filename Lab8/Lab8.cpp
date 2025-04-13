#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <windows.h>
#include <DirectXMath.h>
#include "DDSTextureLoader.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

#define MAX_LOAD_STRING 100
WCHAR windowTitle[MAX_LOAD_STRING] = L"Graphics App by Khamidullin Ilsaf";
WCHAR windowClassName[MAX_LOAD_STRING] = L"GraphicsAppClass";
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ID3D11Buffer* pCullingParametersBuffer = nullptr;
ID3D11Device* pD3DDevice = nullptr;
ID3D11DeviceContext* pDeviceContext = nullptr;
IDXGISwapChain* pSwapChain = nullptr;
ID3D11RenderTargetView* pMainRenderTarget = nullptr;
ID3D11DepthStencilView* pDepthStencil = nullptr;
ID3D11VertexShader* pTransparentVShader = nullptr;
ID3D11VertexShader* pMainVShader = nullptr;
ID3D11PixelShader* pMainPShader = nullptr;
ID3D11InputLayout* pVertexInputLayout = nullptr;
ID3D11Buffer* pVertexDataBuffer = nullptr;
ID3D11Buffer* pModelTransformBuffer = nullptr;
ID3D11Buffer* pViewProjectionBuffer = nullptr;
ID3D11ShaderResourceView* pTextureResource = nullptr;
ID3D11ComputeShader* pCullingComputeShader = nullptr;
ID3D11Buffer* pSceneConstantBuffer = nullptr;
ID3D11Buffer* pIndirectArgsBuffer = nullptr;
ID3D11Buffer* pObjectIDsBuffer = nullptr;
ID3D11UnorderedAccessView* pIndirectArgsAccessView = nullptr;
ID3D11UnorderedAccessView* pObjectIDsAccessView = nullptr;
bool bUseGPUCulling = false;
int iVisibleObjectsGPU = 0;
ID3D11ShaderResourceView* pNormalMapResource = nullptr;
ID3D11SamplerState* pLinearSampler = nullptr;
ID3D11Buffer* pIndirectArgsStaging = nullptr;
ID3D11VertexShader* pSkyboxVShader = nullptr;
ID3D11PixelShader* pSkyboxPShader = nullptr;
ID3D11InputLayout* pSkyboxLayout = nullptr;
ID3D11Buffer* pSkyboxVertexBuffer = nullptr;
ID3D11Buffer* pSkyboxVPBuffer = nullptr;
ID3D11ShaderResourceView* pSkyboxTexture = nullptr;

ID3D11PixelShader* pTransparentPShader = nullptr;
ID3D11Buffer* pTransparentParamsBuffer = nullptr;

ID3D11Buffer* pLightDataBuffer = nullptr;

ID3D11PixelShader* pLightingPShader = nullptr;
ID3D11Buffer* pLightColorBuffer = nullptr;

ID3D11Buffer* pBlueColorParams = nullptr;
ID3D11Buffer* pGreenColorParams = nullptr;

ID3D11BlendState* pAlphaBlendingState = nullptr;
ID3D11DepthStencilState* pTransparentDepthState = nullptr;

bool bApplyPostProcessing = false;

ID3D11Texture2D* pPostProcessTexture = nullptr;
ID3D11RenderTargetView* pPostProcessTarget = nullptr;
ID3D11ShaderResourceView* pPostProcessResource = nullptr;
ID3D11VertexShader* pPostProcessVShader = nullptr;
ID3D11PixelShader* pPostProcessPShader = nullptr;
ID3D11Buffer* pFullscreenQuadBuffer = nullptr;
ID3D11InputLayout* pFullscreenLayout = nullptr;

bool bEnableFrustumCulling = false;
float fObjectRotationAngle = 0.0f;
float fCameraRotationAngle = 0.0f;
bool bIsMouseDragging = false;
POINT ptLastMousePosition = { 0, 0 };
float fCameraHorizontalAngle = 0.0f;
float fCameraVerticalAngle = 0.0f;
int iTotalInstances = 0;
int iRenderedInstances = 0;

#define MAX_GPU_OBJECTS 21
struct CullingParameters
{
    UINT objectCount;
    UINT padding[3];
    XMFLOAT4 boundingBoxMin[MAX_GPU_OBJECTS];
    XMFLOAT4 boundingBoxMax[MAX_GPU_OBJECTS];
};

struct FrustumPlane
{
    float a, b, c, d;
};

void CalculateFrustumPlanes(const XMMATRIX& viewProjMatrix, FrustumPlane planes[6])
{
    planes[0].a = viewProjMatrix.r[0].m128_f32[3] + viewProjMatrix.r[0].m128_f32[0];
    planes[0].b = viewProjMatrix.r[1].m128_f32[3] + viewProjMatrix.r[1].m128_f32[0];
    planes[0].c = viewProjMatrix.r[2].m128_f32[3] + viewProjMatrix.r[2].m128_f32[0];
    planes[0].d = viewProjMatrix.r[3].m128_f32[3] + viewProjMatrix.r[3].m128_f32[0];

    planes[1].a = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[0];
    planes[1].b = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[0];
    planes[1].c = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[0];
    planes[1].d = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[0];

    planes[2].a = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[1];
    planes[2].b = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[1];
    planes[2].c = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[1];
    planes[2].d = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[1];

    planes[3].a = viewProjMatrix.r[0].m128_f32[3] + viewProjMatrix.r[0].m128_f32[1];
    planes[3].b = viewProjMatrix.r[1].m128_f32[3] + viewProjMatrix.r[1].m128_f32[1];
    planes[3].c = viewProjMatrix.r[2].m128_f32[3] + viewProjMatrix.r[2].m128_f32[1];
    planes[3].d = viewProjMatrix.r[3].m128_f32[3] + viewProjMatrix.r[3].m128_f32[1];

    planes[4].a = viewProjMatrix.r[0].m128_f32[2];
    planes[4].b = viewProjMatrix.r[1].m128_f32[2];
    planes[4].c = viewProjMatrix.r[2].m128_f32[2];
    planes[4].d = viewProjMatrix.r[3].m128_f32[2];

    planes[5].a = viewProjMatrix.r[0].m128_f32[3] - viewProjMatrix.r[0].m128_f32[2];
    planes[5].b = viewProjMatrix.r[1].m128_f32[3] - viewProjMatrix.r[1].m128_f32[2];
    planes[5].c = viewProjMatrix.r[2].m128_f32[3] - viewProjMatrix.r[2].m128_f32[2];
    planes[5].d = viewProjMatrix.r[3].m128_f32[3] - viewProjMatrix.r[3].m128_f32[2];
}

bool CheckSphereVisibility(const FrustumPlane planes[6], const XMVECTOR& sphereCenter, float sphereRadius)
{
    for (int i = 0; i < 6; i++)
    {
        float distance = XMVectorGetX(XMVector3Dot(sphereCenter, 
            XMVectorSet(planes[i].a, planes[i].b, planes[i].c, 0.0f))) + planes[i].d;
        if (distance < -sphereRadius)
            return false;
    }
    return true;
}

struct VertexData
{
    float posX, posY, posZ;
    float normX, normY, normZ;
    float texU, texV;
};

struct SceneConstants
{
    XMFLOAT4X4 viewProjection;
    XMFLOAT4 frustumPlanes[6];
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

struct ScreenVertex
{
    float x, y, z, w;
    float u, v;
};

ScreenVertex fullscreenTriangle[3] =
{
    { -1.0f, -1.0f, 0, 1,   0.0f, 1.0f },
    { -1.0f,  3.0f, 0, 1,   0.0f,-1.0f },
    {  3.0f, -1.0f, 0, 1,   2.0f, 1.0f }
};

struct SkyVertex
{
    float x, y, z;
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
void CleanupDirect3D();
void RenderFrame();

struct LightParameters
{
    XMFLOAT3 lightPosition0;
    float padding0;
    XMFLOAT3 lightColor0;
    float padding1;
    XMFLOAT3 lightPosition1;
    float padding2;
    XMFLOAT3 lightColor1;
    float padding3;
    XMFLOAT3 ambientLight;
    float padding4;
};

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
    const D3D_FEATURE_LEVEL supportedLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        creationFlags, supportedLevels, 1,
        D3D11_SDK_VERSION, &swapDesc, &pSwapChain,
        &pD3DDevice, &featureLevel, &pDeviceContext);
    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* pBackBufferTexture = nullptr;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferTexture);
    if (FAILED(hr))
        return hr;
    hr = pD3DDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &pMainRenderTarget);
    pBackBufferTexture->Release();
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
    ID3D11Texture2D* pDepthTexture = nullptr;
    hr = pD3DDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthTexture);
    if (FAILED(hr))
        return hr;
    hr = pD3DDevice->CreateDepthStencilView(pDepthTexture, nullptr, &pDepthStencil);
    pDepthTexture->Release();
    if (FAILED(hr))
        return hr;

    pDeviceContext->OMSetRenderTargets(1, &pMainRenderTarget, pDepthStencil);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<FLOAT>(width);
    viewport.Height = static_cast<FLOAT>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    pDeviceContext->RSSetViewports(1, &viewport);

    D3D11_TEXTURE2D_DESC postProcessDesc = {};
    postProcessDesc.Width = width;
    postProcessDesc.Height = height;
    postProcessDesc.MipLevels = 1;
    postProcessDesc.ArraySize = 1;
    postProcessDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    postProcessDesc.SampleDesc.Count = 1;
    postProcessDesc.Usage = D3D11_USAGE_DEFAULT;
    postProcessDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    hr = pD3DDevice->CreateTexture2D(&postProcessDesc, nullptr, &pPostProcessTexture);
    if (FAILED(hr))
        return hr;

    hr = pD3DDevice->CreateRenderTargetView(pPostProcessTexture, nullptr, &pPostProcessTarget);
    if (FAILED(hr))
        return hr;

    hr = pD3DDevice->CreateShaderResourceView(pPostProcessTexture, nullptr, &pPostProcessResource);
    if (FAILED(hr))
        return hr;

    return S_OK;
}
HRESULT InitializeResources()
{
    HRESULT hr = S_OK;
    ID3DBlob* pShaderBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pVertexShaderBlob = nullptr;
    ID3DBlob* pPixelShaderBlob = nullptr;

    hr = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Vertex Shader Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pMainVShader);
    if (FAILED(hr))
    {
        pShaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                        D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,           D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(float) * 6,           D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = pD3DDevice->CreateInputLayout(vertexLayoutDesc, 3, pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &pVertexInputLayout);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Pixel Shader Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pMainPShader);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VertexData) * ARRAYSIZE(cubeVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = cubeVertices;
    hr = pD3DDevice->CreateBuffer(&bufferDesc, &initData, &pVertexDataBuffer);
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    const int numInstances = 21;
    bufferDesc.ByteWidth = sizeof(XMMATRIX) * numInstances;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    hr = pD3DDevice->CreateBuffer(&bufferDesc, nullptr, &pModelTransformBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC viewProjBufferDesc = {};
    viewProjBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    viewProjBufferDesc.ByteWidth = sizeof(XMMATRIX);
    viewProjBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    viewProjBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = pD3DDevice->CreateBuffer(&viewProjBufferDesc, nullptr, &pViewProjectionBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pD3DDevice, L"cube.dds", nullptr, &pTextureResource);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pD3DDevice, L"cube_normal.dds", nullptr, &pNormalMapResource);
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
    hr = pD3DDevice->CreateSamplerState(&samplerDesc, &pLinearSampler);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Skybox_VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Skybox VS Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pSkyboxVShader);
    if (FAILED(hr))
    {
        pShaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC skyboxLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = pD3DDevice->CreateInputLayout(skyboxLayoutDesc, 1, pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &pSkyboxLayout);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Skybox_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Skybox PS Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pSkyboxPShader);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(SkyVertex) * ARRAYSIZE(skyboxVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyboxVertices;
    hr = pD3DDevice->CreateBuffer(&bufferDesc, &initData, &pSkyboxVertexBuffer);
    if (FAILED(hr))
        return hr;

    viewProjBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    viewProjBufferDesc.ByteWidth = sizeof(XMMATRIX);
    viewProjBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    viewProjBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = pD3DDevice->CreateBuffer(&viewProjBufferDesc, nullptr, &pSkyboxVPBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(pD3DDevice, L"skybox.dds", nullptr, &pSkyboxTexture);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Transparent_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Transparent PS Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pTransparentPShader);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC transparentBufferDesc = {};
    transparentBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    transparentBufferDesc.ByteWidth = sizeof(XMFLOAT4);
    transparentBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    transparentBufferDesc.CPUAccessFlags = 0;
    hr = pD3DDevice->CreateBuffer(&transparentBufferDesc, nullptr, &pTransparentParamsBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufferDesc.ByteWidth = sizeof(LightParameters);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = 0;
    hr = pD3DDevice->CreateBuffer(&lightBufferDesc, nullptr, &pLightDataBuffer);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"Light_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Light PS Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pLightingPShader);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightColorDesc = {};
    lightColorDesc.Usage = D3D11_USAGE_DEFAULT;
    lightColorDesc.ByteWidth = sizeof(XMFLOAT4);
    lightColorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightColorDesc.CPUAccessFlags = 0;
    hr = pD3DDevice->CreateBuffer(&lightColorDesc, nullptr, &pLightColorBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC colorBufferDesc = {};
    colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    colorBufferDesc.ByteWidth = sizeof(XMFLOAT4);
    colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colorBufferDesc.CPUAccessFlags = 0;

    hr = pD3DDevice->CreateBuffer(&colorBufferDesc, nullptr, &pBlueColorParams);
    if (FAILED(hr)) return hr;

    hr = pD3DDevice->CreateBuffer(&colorBufferDesc, nullptr, &pGreenColorParams);
    if (FAILED(hr)) return hr;

    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = pD3DDevice->CreateBlendState(&blendStateDesc, &pAlphaBlendingState);
    if (FAILED(hr))
        return hr;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &pTransparentDepthState);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"transparent_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Transparent VS Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
        nullptr, &pTransparentVShader);
    pShaderBlob->Release();
    if (FAILED(hr))
        return hr;

    ID3DBlob* pPostProcessVSBlob = nullptr;
    ID3DBlob* pPostProcessPSBlob = nullptr;
    ID3DBlob* pPostProcessErrorBlob = nullptr;

    hr = D3DCompileFromFile(
        L"PostProcessVS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        0, 0,
        &pPostProcessVSBlob,
        &pPostProcessErrorBlob
    );
    if (FAILED(hr))
    {
        if (pPostProcessErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pPostProcessErrorBlob->GetBufferPointer(),
                "PostProcess VS Compile Error", MB_OK);
            pPostProcessErrorBlob->Release();
        }
        return hr;
    }

    hr = pD3DDevice->CreateVertexShader(
        pPostProcessVSBlob->GetBufferPointer(),
        pPostProcessVSBlob->GetBufferSize(),
        nullptr,
        &pPostProcessVShader
    );
    if (FAILED(hr))
    {
        pPostProcessVSBlob->Release();
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC postProcessLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(float) * 4,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = pD3DDevice->CreateInputLayout(
        postProcessLayoutDesc,
        _countof(postProcessLayoutDesc),
        pPostProcessVSBlob->GetBufferPointer(),
        pPostProcessVSBlob->GetBufferSize(),
        &pFullscreenLayout
    );
    if (FAILED(hr))
    {
        pPostProcessVSBlob->Release();
        return hr;
    }

    pPostProcessVSBlob->Release();
    pPostProcessVSBlob = nullptr;

    hr = D3DCompileFromFile(
        L"PostProcessPS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        0, 0,
        &pPostProcessPSBlob,
        &pPostProcessErrorBlob
    );
    if (FAILED(hr))
    {
        if (pPostProcessErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pPostProcessErrorBlob->GetBufferPointer(),
                "PostProcess PS Compile Error", MB_OK);
            pPostProcessErrorBlob->Release();
        }
        return hr;
    }

    hr = pD3DDevice->CreatePixelShader(
        pPostProcessPSBlob->GetBufferPointer(),
        pPostProcessPSBlob->GetBufferSize(),
        nullptr,
        &pPostProcessPShader
    );

    pPostProcessPSBlob->Release();
    pPostProcessPSBlob = nullptr;

    if (FAILED(hr))
    {
        return hr;
    }

    {
        D3D11_BUFFER_DESC fullscreenBufferDesc = {};
        fullscreenBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        fullscreenBufferDesc.ByteWidth = sizeof(ScreenVertex) * 3;
        fullscreenBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        fullscreenBufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA fullscreenInitData = {};
        fullscreenInitData.pSysMem = fullscreenTriangle;

        hr = pD3DDevice->CreateBuffer(&fullscreenBufferDesc, &fullscreenInitData, &pFullscreenQuadBuffer);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    ID3DBlob* pComputeShaderBlob = nullptr;
    ID3DBlob* pComputeErrorBlob = nullptr;
    hr = D3DCompileFromFile(L"GpuCulling.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "cs_5_0", 0, 0, &pComputeShaderBlob, &pComputeErrorBlob);
    if (FAILED(hr))
    {
        if (pComputeErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pComputeErrorBlob->GetBufferPointer(), "Compute Shader Compile Error", MB_OK);
            pComputeErrorBlob->Release();
        }
        return hr;
    }
    hr = pD3DDevice->CreateComputeShader(pComputeShaderBlob->GetBufferPointer(), pComputeShaderBlob->GetBufferSize(), nullptr, &pCullingComputeShader);
    pComputeShaderBlob->Release();
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_BUFFER_DESC indirectBufferDesc = {};
    indirectBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indirectBufferDesc.ByteWidth = sizeof(UINT) * 4;
    indirectBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    indirectBufferDesc.CPUAccessFlags = 0;
    indirectBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    indirectBufferDesc.StructureByteStride = sizeof(UINT);

    hr = pD3DDevice->CreateBuffer(&indirectBufferDesc, nullptr, &pIndirectArgsBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = 4;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;

    hr = pD3DDevice->CreateUnorderedAccessView(pIndirectArgsBuffer, &uavDesc, &pIndirectArgsAccessView);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC cullingParamsDesc = {};
    cullingParamsDesc.Usage = D3D11_USAGE_DEFAULT;
    cullingParamsDesc.ByteWidth = sizeof(CullingParameters);
    cullingParamsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cullingParamsDesc.CPUAccessFlags = 0;
    cullingParamsDesc.MiscFlags = 0;

    hr = pD3DDevice->CreateBuffer(&cullingParamsDesc, nullptr, &pCullingParametersBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_BUFFER_DESC stagingBufferDesc = {};
    stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
    stagingBufferDesc.ByteWidth = sizeof(UINT) * 4;
    stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingBufferDesc.BindFlags = 0;
    stagingBufferDesc.MiscFlags = 0;
    hr = pD3DDevice->CreateBuffer(&stagingBufferDesc, nullptr, &pIndirectArgsStaging);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC sceneBufferDesc = {};
    sceneBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    sceneBufferDesc.ByteWidth = sizeof(SceneConstants);
    sceneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    sceneBufferDesc.CPUAccessFlags = 0;
    sceneBufferDesc.MiscFlags = 0;

    hr = pD3DDevice->CreateBuffer(&sceneBufferDesc, nullptr, &pSceneConstantBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(FindWindow(windowClassName, windowTitle));

    ImGui_ImplDX11_Init(pD3DDevice, pDeviceContext);

    return S_OK;
}

void CleanupDirect3D()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (pDeviceContext)
        pDeviceContext->ClearState();

    if (pVertexDataBuffer)        pVertexDataBuffer->Release();
    if (pVertexInputLayout)       pVertexInputLayout->Release();
    if (pMainVShader)             pMainVShader->Release();
    if (pMainPShader)             pMainPShader->Release();
    if (pModelTransformBuffer)    pModelTransformBuffer->Release();
    if (pViewProjectionBuffer)    pViewProjectionBuffer->Release();
    if (pTextureResource)         pTextureResource->Release();

    if (pNormalMapResource)       pNormalMapResource->Release();
    if (pLinearSampler)           pLinearSampler->Release();
    if (pMainRenderTarget)        pMainRenderTarget->Release();
    if (pDepthStencil)            pDepthStencil->Release();
    if (pSwapChain)               pSwapChain->Release();
    if (pDeviceContext)           pDeviceContext->Release();
    if (pD3DDevice)               pD3DDevice->Release();

    if (pSkyboxVertexBuffer)      pSkyboxVertexBuffer->Release();
    if (pSkyboxLayout)            pSkyboxLayout->Release();
    if (pSkyboxVShader)           pSkyboxVShader->Release();
    if (pSkyboxPShader)           pSkyboxPShader->Release();
    if (pSkyboxVPBuffer)          pSkyboxVPBuffer->Release();
    if (pSkyboxTexture)           pSkyboxTexture->Release();

    if (pTransparentPShader)      pTransparentPShader->Release();
    if (pTransparentParamsBuffer) pTransparentParamsBuffer->Release();
    if (pAlphaBlendingState)      pAlphaBlendingState->Release();
    if (pTransparentDepthState)   pTransparentDepthState->Release();

    if (pLightDataBuffer)         pLightDataBuffer->Release();
    if (pLightingPShader)         pLightingPShader->Release();
    if (pLightColorBuffer)        pLightColorBuffer->Release();
    if (pBlueColorParams)         pBlueColorParams->Release();
    if (pGreenColorParams)        pGreenColorParams->Release();
    if (pTransparentVShader)      pTransparentVShader->Release();

    if (pPostProcessTexture)      pPostProcessTexture->Release();
    if (pPostProcessTarget)       pPostProcessTarget->Release();
    if (pPostProcessResource)     pPostProcessResource->Release();
    if (pPostProcessVShader)      pPostProcessVShader->Release();
    if (pPostProcessPShader)      pPostProcessPShader->Release();

    if (pFullscreenQuadBuffer)    pFullscreenQuadBuffer->Release();
    if (pFullscreenLayout)        pFullscreenLayout->Release();
    if (pCullingComputeShader)    pCullingComputeShader->Release();
    if (pCullingParametersBuffer) pCullingParametersBuffer->Release();
    if (pIndirectArgsBuffer)      pIndirectArgsBuffer->Release();
    if (pIndirectArgsAccessView)  pIndirectArgsAccessView->Release();
    if (pObjectIDsBuffer)         pObjectIDsBuffer->Release();
    if (pObjectIDsAccessView)     pObjectIDsAccessView->Release();
    if (pIndirectArgsStaging)     pIndirectArgsStaging->Release();
    if (pSceneConstantBuffer)     pSceneConstantBuffer->Release();
}

void RenderScene(bool renderToBackbuffer)
{
    if (renderToBackbuffer)
    {
        pDeviceContext->OMSetRenderTargets(1, &pMainRenderTarget, pDepthStencil);
        float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
        pDeviceContext->ClearRenderTargetView(pMainRenderTarget, clearColor);
        pDeviceContext->ClearDepthStencilView(pDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    RECT clientRect;
    GetClientRect(FindWindow(windowClassName, windowTitle), &clientRect);
    float aspectRatio = static_cast<float>(clientRect.right - clientRect.left) / (clientRect.bottom - clientRect.top);
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

    float cameraDistance = 6.0f;
    float cameraX = cameraDistance * sinf(fCameraHorizontalAngle) * cosf(fCameraVerticalAngle);
    float cameraY = cameraDistance * sinf(fCameraVerticalAngle);
    float cameraZ = cameraDistance * cosf(fCameraHorizontalAngle) * cosf(fCameraVerticalAngle);
    XMVECTOR cameraPosition = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    XMVECTOR lookAtPoint = XMVectorZero();
    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(cameraPosition, lookAtPoint, upVector);

    XMMATRIX skyboxViewMatrix = viewMatrix;
    skyboxViewMatrix.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX skyboxViewProjection = XMMatrixTranspose(skyboxViewMatrix * projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(pDeviceContext->Map(pSkyboxVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        memcpy(mappedResource.pData, &skyboxViewProjection, sizeof(XMMATRIX));
        pDeviceContext->Unmap(pSkyboxVPBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* pSkyboxDepthState = nullptr;
    pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &pSkyboxDepthState);
    pDeviceContext->OMSetDepthStencilState(pSkyboxDepthState, 0);

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_FRONT;
    rasterizerDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* pSkyboxRasterizer = nullptr;
    if (SUCCEEDED(pD3DDevice->CreateRasterizerState(&rasterizerDesc, &pSkyboxRasterizer)))
    {
        pDeviceContext->RSSetState(pSkyboxRasterizer);
    }

    UINT vertexStride = sizeof(SkyVertex);
    UINT vertexOffset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pSkyboxVertexBuffer, &vertexStride, &vertexOffset);
    pDeviceContext->IASetInputLayout(pSkyboxLayout);
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pDeviceContext->VSSetShader(pSkyboxVShader, nullptr, 0);
    pDeviceContext->VSSetConstantBuffers(0, 1, &pSkyboxVPBuffer);
    pDeviceContext->PSSetShader(pSkyboxPShader, nullptr, 0);
    pDeviceContext->PSSetShaderResources(0, 1, &pSkyboxTexture);
    pDeviceContext->PSSetSamplers(0, 1, &pLinearSampler);
    pDeviceContext->Draw(ARRAYSIZE(skyboxVertices), 0);

    pSkyboxDepthState->Release();
    if (pSkyboxRasterizer)
    {
        pSkyboxRasterizer->Release();
        pDeviceContext->RSSetState(nullptr);
    }
    pDeviceContext->OMSetDepthStencilState(nullptr, 0);

    fObjectRotationAngle += 0.01f;

    XMMATRIX cubeViewProjection = XMMatrixTranspose(viewMatrix * projectionMatrix);
    if (SUCCEEDED(pDeviceContext->Map(pViewProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        memcpy(mappedResource.pData, &cubeViewProjection, sizeof(XMMATRIX));
        pDeviceContext->Unmap(pViewProjectionBuffer, 0);
    }

    const int nearInstances = 11;
    const int farInstances = 10;
    const int totalInstances = nearInstances + farInstances;
    iTotalInstances = totalInstances;

    XMMATRIX instanceTransforms[totalInstances];

    instanceTransforms[0] = XMMatrixRotationY(fObjectRotationAngle);
    float nearRadius = 3.0f;
    float nearAngleStep = XM_2PI / 10.0f;
    for (int i = 1; i < nearInstances; i++)
    {
        float angleOffset = (i - 1) * nearAngleStep;
        float orbitAngle = fObjectRotationAngle + angleOffset;
        XMMATRIX translation = XMMatrixTranslation(nearRadius * cosf(orbitAngle), 0.0f, nearRadius * sinf(orbitAngle));
        XMMATRIX rotation = XMMatrixRotationY(fObjectRotationAngle);
        instanceTransforms[i] = translation * rotation;
    }

    float farRadius = 40.0f;
    float farAngleStep = XM_2PI / farInstances;
    for (int i = nearInstances; i < totalInstances; i++)
    {
        float angleOffset = (i - nearInstances) * farAngleStep;
        float orbitAngle = fObjectRotationAngle + angleOffset;
        XMMATRIX translation = XMMatrixTranslation(farRadius * cosf(orbitAngle), 0.0f, farRadius * sinf(orbitAngle));
        XMMATRIX rotation = XMMatrixRotationY(fObjectRotationAngle);
        instanceTransforms[i] = translation * rotation;
    }

    XMMATRIX viewProjMatrix = viewMatrix * projectionMatrix;
    FrustumPlane frustumPlanes[6];
    CalculateFrustumPlanes(viewProjMatrix, frustumPlanes);
    for (int i = 0; i < 6; i++)
    {
        float length = sqrtf(frustumPlanes[i].a * frustumPlanes[i].a +
            frustumPlanes[i].b * frustumPlanes[i].b +
            frustumPlanes[i].c * frustumPlanes[i].c);
        frustumPlanes[i].a /= length;
        frustumPlanes[i].b /= length;
        frustumPlanes[i].c /= length;
        frustumPlanes[i].d /= length;
    }

    if (bUseGPUCulling)
    {
        CullingParameters cullingParams = {};
        cullingParams.objectCount = totalInstances;
        for (int i = 0; i < totalInstances && i < MAX_GPU_OBJECTS; i++)
        {
            XMVECTOR center = instanceTransforms[i].r[3];
            XMVECTOR offset = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
            XMVECTOR minVec = XMVectorSubtract(center, offset);
            XMVECTOR maxVec = XMVectorAdd(center, offset);
            XMStoreFloat4(&cullingParams.boundingBoxMin[i], minVec);
            XMStoreFloat4(&cullingParams.boundingBoxMax[i], maxVec);
        }
        pDeviceContext->UpdateSubresource(pCullingParametersBuffer, 0, nullptr, &cullingParams, 0, 0);

        SceneConstants sceneConstants;
        XMStoreFloat4x4(
            &sceneConstants.viewProjection,
            XMMatrixTranspose(viewMatrix * projectionMatrix)
        );
        for (int i = 0; i < 6; i++)
        {
            sceneConstants.frustumPlanes[i] = XMFLOAT4(
                frustumPlanes[i].a,
                frustumPlanes[i].b,
                frustumPlanes[i].c,
                frustumPlanes[i].d
            );
        }

        pDeviceContext->UpdateSubresource(pSceneConstantBuffer, 0, nullptr, &sceneConstants, 0, 0);
        pDeviceContext->CSSetConstantBuffers(1, 1, &pSceneConstantBuffer);

        UINT clearValues[4] = { 0, 0, 0, 0 };
        pDeviceContext->ClearUnorderedAccessViewUint(pIndirectArgsAccessView, clearValues);

        pDeviceContext->CSSetShader(pCullingComputeShader, nullptr, 0);
        pDeviceContext->CSSetConstantBuffers(0, 1, &pCullingParametersBuffer);
        pDeviceContext->CSSetUnorderedAccessViews(0, 1, &pIndirectArgsAccessView, nullptr);
        pDeviceContext->CSSetUnorderedAccessViews(1, 1, &pObjectIDsAccessView, nullptr);

        UINT dispatchGroups = (totalInstances + 63) / 64;
        pDeviceContext->Dispatch(dispatchGroups, 1, 1);

        ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
        pDeviceContext->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
        pDeviceContext->CSSetShader(nullptr, nullptr, 0);

        pDeviceContext->CopyResource(pIndirectArgsStaging, pIndirectArgsBuffer);

        D3D11_MAPPED_SUBRESOURCE mappedData = {};
        HRESULT hr = pDeviceContext->Map(pIndirectArgsStaging, 0, D3D11_MAP_READ, 0, &mappedData);
        UINT visibleObjects = 0;
        if (SUCCEEDED(hr))
        {
            UINT* data = (UINT*)mappedData.pData;
            visibleObjects = data[1];
            pDeviceContext->Unmap(pIndirectArgsStaging, 0);
        }
        iVisibleObjectsGPU = visibleObjects;

        XMMATRIX transposedTransforms[totalInstances];
        for (int i = 0; i < totalInstances; i++)
        {
            transposedTransforms[i] = XMMatrixTranspose(instanceTransforms[i]);
        }
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, transposedTransforms, 0, 0);

        pDeviceContext->DrawInstanced(ARRAYSIZE(cubeVertices), visibleObjects, 0, 0);
    }
    else
    {
        XMMATRIX visibleTransforms[totalInstances];
        int visibleCount = 0;
        if (bEnableFrustumCulling)
        {
            for (int i = 0; i < totalInstances; i++)
            {
                XMVECTOR center = XMVectorSet(instanceTransforms[i].r[3].m128_f32[0],
                    instanceTransforms[i].r[3].m128_f32[1],
                    instanceTransforms[i].r[3].m128_f32[2],
                    1.0f);
                if (CheckSphereVisibility(frustumPlanes, center, 1.0f))
                {
                    visibleTransforms[visibleCount++] = instanceTransforms[i];
                }
            }
        }
        else
        {
            memcpy(visibleTransforms, instanceTransforms, sizeof(instanceTransforms));
            visibleCount = totalInstances;
        }
        iRenderedInstances = visibleCount;
        iVisibleObjectsGPU = totalInstances;
        XMMATRIX transposedVisibleTransforms[totalInstances];
        for (int i = 0; i < visibleCount; i++)
        {
            transposedVisibleTransforms[i] = XMMatrixTranspose(visibleTransforms[i]);
        }
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, transposedVisibleTransforms, 0, 0);
        pDeviceContext->DrawInstanced(ARRAYSIZE(cubeVertices), visibleCount, 0, 0);
    }

    vertexStride = sizeof(VertexData);
    vertexOffset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pVertexDataBuffer, &vertexStride, &vertexOffset);
    pDeviceContext->IASetInputLayout(pVertexInputLayout);
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pDeviceContext->VSSetShader(pMainVShader, nullptr, 0);
    pDeviceContext->PSSetShader(pMainPShader, nullptr, 0);
    pDeviceContext->VSSetConstantBuffers(0, 1, &pModelTransformBuffer);
    pDeviceContext->VSSetConstantBuffers(1, 1, &pViewProjectionBuffer);
    pDeviceContext->PSSetConstantBuffers(0, 1, &pLightDataBuffer);

    ID3D11ShaderResourceView* textureViews[2] = { pTextureResource, pNormalMapResource };
    pDeviceContext->PSSetShaderResources(0, 2, textureViews);
    pDeviceContext->PSSetSamplers(0, 1, &pLinearSampler);

    pDeviceContext->DrawInstanced(ARRAYSIZE(cubeVertices), iRenderedInstances, 0, 0);

    float blendFactors[4] = { 0, 0, 0, 0 };
    pDeviceContext->OMSetBlendState(pAlphaBlendingState, blendFactors, 0xFFFFFFFF);
    pDeviceContext->OMSetDepthStencilState(pTransparentDepthState, 0);
    ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
    pDeviceContext->PSSetShaderResources(0, 1, &pTextureResource);

    static float blueAnimation = 0.0f;
    blueAnimation += 0.02f;
    float blueZOffset = 2.0f * sinf(blueAnimation);
    XMMATRIX blueTransform = XMMatrixTranslation(-2.0f, 0.0f, blueZOffset);
    XMVECTOR bluePosition = XMVectorSet(-2.0f, 0.0f, blueZOffset, 1.0f);

    static float greenAnimation = 0.0f;
    greenAnimation += 0.02f;
    float greenYOffset = 2.0f * sinf(greenAnimation);
    XMMATRIX greenTransform = XMMatrixTranslation(2.0f, greenYOffset, 0.0f);
    XMVECTOR greenPosition = XMVectorSet(2.0f, greenYOffset, 0.0f, 1.0f);

    float blueDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(bluePosition, cameraPosition)));
    float greenDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(greenPosition, cameraPosition)));

    pDeviceContext->PSSetShader(pTransparentPShader, nullptr, 0);
    pDeviceContext->VSSetShader(pTransparentVShader, nullptr, 0);
    pDeviceContext->PSSetConstantBuffers(0, 1, &pLightDataBuffer);

    if (blueDistance >= greenDistance)
    {
        XMMATRIX transposedBlue = XMMatrixTranspose(blueTransform);
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &transposedBlue, 0, 0);
        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        pDeviceContext->UpdateSubresource(pBlueColorParams, 0, nullptr, &blueColor, 0, 0);
        pDeviceContext->PSSetConstantBuffers(1, 1, &pBlueColorParams);
        pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX transposedGreen = XMMatrixTranspose(greenTransform);
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &transposedGreen, 0, 0);
        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        pDeviceContext->UpdateSubresource(pGreenColorParams, 0, nullptr, &greenColor, 0, 0);
        pDeviceContext->PSSetConstantBuffers(1, 1, &pGreenColorParams);
        pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }
    else
    {
        XMMATRIX transposedGreen = XMMatrixTranspose(greenTransform);
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &transposedGreen, 0, 0);
        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        pDeviceContext->UpdateSubresource(pGreenColorParams, 0, nullptr, &greenColor, 0, 0);
        pDeviceContext->PSSetConstantBuffers(1, 1, &pGreenColorParams);
        pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX transposedBlue = XMMatrixTranspose(blueTransform);
        pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &transposedBlue, 0, 0);
        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        pDeviceContext->UpdateSubresource(pBlueColorParams, 0, nullptr, &blueColor, 0, 0);
        pDeviceContext->PSSetConstantBuffers(1, 1, &pBlueColorParams);
        pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }

    pDeviceContext->OMSetBlendState(nullptr, blendFactors, 0xFFFFFFFF);
    pDeviceContext->OMSetDepthStencilState(nullptr, 0);

    LightParameters lightParams;
    lightParams.lightPosition0 = XMFLOAT3(1.0f, 0.0f, 0.0f);
    lightParams.lightColor0 = XMFLOAT3(3.0f, 3.0f, 0.0f);
    lightParams.lightPosition1 = XMFLOAT3(-1.0f, 0.0f, 0.0f);
    lightParams.lightColor1 = XMFLOAT3(3.0f, 3.0f, 3.0f);
    lightParams.ambientLight = XMFLOAT3(0.0f, 0.0f, 0.0f);
    pDeviceContext->UpdateSubresource(pLightDataBuffer, 0, nullptr, &lightParams, 0, 0);

    pDeviceContext->PSSetShader(pLightingPShader, nullptr, 0);
    pDeviceContext->PSSetConstantBuffers(0, 1, &pLightColorBuffer);

    XMMATRIX lightScaleMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);

    XMMATRIX lightTranslate0 = XMMatrixTranslation(1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel0 = XMMatrixTranspose(lightScaleMatrix * lightTranslate0);
    pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &lightModel0, 0, 0);
    XMFLOAT4 lightColor0(1.0f, 1.0f, 0.0f, 1.0f);
    pDeviceContext->UpdateSubresource(pLightColorBuffer, 0, nullptr, &lightColor0, 0, 0);
    pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    XMMATRIX lightTranslate1 = XMMatrixTranslation(-1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel1 = XMMatrixTranspose(lightScaleMatrix * lightTranslate1);
    pDeviceContext->UpdateSubresource(pModelTransformBuffer, 0, nullptr, &lightModel1, 0, 0);
    XMFLOAT4 lightColor1(1.0f, 1.0f, 1.0f, 1.0f);
    pDeviceContext->UpdateSubresource(pLightColorBuffer, 0, nullptr, &lightColor1, 0, 0);
    pDeviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
}

void RenderFrame()
{
    float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

    if (bApplyPostProcessing)
    {
        pDeviceContext->OMSetRenderTargets(1, &pPostProcessTarget, pDepthStencil);
        pDeviceContext->ClearRenderTargetView(pPostProcessTarget, clearColor);
        pDeviceContext->ClearDepthStencilView(pDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
        RenderScene(false);
    }
    else
    {
        pDeviceContext->OMSetRenderTargets(1, &pMainRenderTarget, pDepthStencil);
        pDeviceContext->ClearRenderTargetView(pMainRenderTarget, clearColor);
        pDeviceContext->ClearDepthStencilView(pDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
        RenderScene(true);
    }
    if (bApplyPostProcessing)
    {
        pDeviceContext->OMSetRenderTargets(1, &pMainRenderTarget, nullptr);
        pDeviceContext->OMSetDepthStencilState(nullptr, 0);

        UINT stride = sizeof(ScreenVertex);
        UINT offset = 0;
        pDeviceContext->IASetVertexBuffers(0, 1, &pFullscreenQuadBuffer, &stride, &offset);
        pDeviceContext->IASetInputLayout(pFullscreenLayout);
        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        pDeviceContext->VSSetShader(pPostProcessVShader, nullptr, 0);
        pDeviceContext->PSSetShader(pPostProcessPShader, nullptr, 0);

        pDeviceContext->PSSetShaderResources(0, 1, &pPostProcessResource);
        pDeviceContext->PSSetSamplers(0, 1, &pLinearSampler);

        pDeviceContext->Draw(3, 0);

        ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
        pDeviceContext->PSSetShaderResources(0, 1, nullSRVs);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(60, 40), ImGuiCond_FirstUseEver);

    ImGui::Begin("Options");
    ImGui::Checkbox("Grayscale Filter", &bApplyPostProcessing);
    ImGui::Checkbox("Frustum Culling (CPU)", &bEnableFrustumCulling);
    ImGui::Checkbox("GPU Culling", &bUseGPUCulling);
    ImGui::Text("Total cubes: %d", iTotalInstances);
    ImGui::Text("Visible cubes (CPU): %d", iRenderedInstances);
    ImGui::Text("Visible cubes (GPU): %d", iVisibleObjectsGPU);
    ImGui::End();

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    pSwapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        bIsMouseDragging = true;
        ptLastMousePosition.x = LOWORD(lParam);
        ptLastMousePosition.y = HIWORD(lParam);
        SetCapture(hWnd);
        break;
    case WM_LBUTTONUP:
        bIsMouseDragging = false;
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        if (bIsMouseDragging)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int dx = x - ptLastMousePosition.x;
            int dy = y - ptLastMousePosition.y;
            fCameraHorizontalAngle += dx * 0.005f;
            fCameraVerticalAngle += dy * 0.005f;
            if (fCameraVerticalAngle > XM_PIDIV2 - 0.01f)
                fCameraVerticalAngle = XM_PIDIV2 - 0.01f;
            if (fCameraVerticalAngle < -XM_PIDIV2 + 0.01f)
                fCameraVerticalAngle = -XM_PIDIV2 + 0.01f;
            ptLastMousePosition.x = x;
            ptLastMousePosition.y = y;
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