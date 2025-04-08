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

#define MAX_STRING_LENGTH 100
WCHAR windowTitle[MAX_STRING_LENGTH] = L"GraphicsLab Khamidullin Ilsaf";
WCHAR windowClassName[MAX_STRING_LENGTH] = L"GraphicsLabClass";
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

ID3D11Device* graphicsDevice = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTarget = nullptr;
ID3D11DepthStencilView* depthBuffer = nullptr;
ID3D11VertexShader* transparentVertexShader = nullptr;
ID3D11VertexShader* mainVertexShader = nullptr;
ID3D11PixelShader* mainPixelShader = nullptr;
ID3D11InputLayout* inputLayout = nullptr;
ID3D11Buffer* vertexData = nullptr;
ID3D11Buffer* modelBuffer = nullptr;
ID3D11Buffer* viewProjectionBuffer = nullptr;
ID3D11ShaderResourceView* cubeTexture = nullptr;

ID3D11ShaderResourceView* cubeNormalMap = nullptr;
ID3D11SamplerState* textureSampler = nullptr;

ID3D11VertexShader* skyVertexShader = nullptr;
ID3D11PixelShader* skyPixelShader = nullptr;
ID3D11InputLayout* skyInputLayout = nullptr;
ID3D11Buffer* skyVertexData = nullptr;
ID3D11Buffer* skyViewProjBuffer = nullptr;
ID3D11ShaderResourceView* skyTexture = nullptr;

ID3D11PixelShader* transparentPixelShader = nullptr;
ID3D11Buffer* transparentDataBuffer = nullptr;

ID3D11Buffer* lightingDataBuffer = nullptr;

ID3D11PixelShader* lightEffectShader = nullptr;
ID3D11Buffer* lightColorData = nullptr;

ID3D11Buffer* blueLightBuffer = nullptr;
ID3D11Buffer* greenLightBuffer = nullptr;

ID3D11BlendState* blendState = nullptr;
ID3D11DepthStencilState* depthStateTransparent = nullptr;

bool enableImageFilter = false;

ID3D11Texture2D* processedTexture = nullptr;
ID3D11RenderTargetView* processedTarget = nullptr;
ID3D11ShaderResourceView* processedResource = nullptr;
ID3D11VertexShader* postProcessVS = nullptr;
ID3D11PixelShader* postProcessPS = nullptr;
ID3D11Buffer* screenQuadBuffer = nullptr;
ID3D11InputLayout* screenLayout = nullptr;

bool enableViewCulling = false;
float cubeRotationAngle = 0.0f;
float cameraRotationAngle = 0.0f;
bool isMouseDragging = false;
POINT previousMousePosition = { 0, 0 };
float cameraHorizontalAngle = 0.0f;
float cameraVerticalAngle = 0.0f;
int totalObjects = 0;
int visibleObjects = 0;

struct FrustumPlane
{
    float x, y, z, w;
};

void GetFrustumPlanes(const XMMATRIX& matrix, FrustumPlane planes[6])
{
    planes[0].x = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[0];
    planes[0].y = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[0];
    planes[0].z = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[0];
    planes[0].w = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[0];

    planes[1].x = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[0];
    planes[1].y = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[0];
    planes[1].z = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[0];
    planes[1].w = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[0];

    planes[2].x = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[1];
    planes[2].y = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[1];
    planes[2].z = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[1];
    planes[2].w = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[1];

    planes[3].x = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[1];
    planes[3].y = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[1];
    planes[3].z = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[1];
    planes[3].w = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[1];

    planes[4].x = matrix.r[0].m128_f32[2];
    planes[4].y = matrix.r[1].m128_f32[2];
    planes[4].z = matrix.r[2].m128_f32[2];
    planes[4].w = matrix.r[3].m128_f32[2];

    planes[5].x = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[2];
    planes[5].y = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[2];
    planes[5].z = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[2];
    planes[5].w = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[2];
}

bool CheckSphereVisibility(const FrustumPlane planes[6], const XMVECTOR& sphereCenter, float sphereRadius)
{
    for (int i = 0; i < 6; i++)
    {
        float distance = XMVectorGetX(XMVector3Dot(sphereCenter, XMVectorSet(planes[i].x, planes[i].y, planes[i].z, 0.0f))) + planes[i].w;
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

ATOM RegisterWindowClass(HINSTANCE appInstance);
BOOL InitializeWindow(HINSTANCE, int);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
HRESULT InitializeDirect3D(HWND windowHandle);
HRESULT InitializeGraphicsResources();
void ReleaseDirect3DResources();
void DrawScene();

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

int APIENTRY wWinMain(HINSTANCE appInstance, HINSTANCE, LPWSTR, int showCommand)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    RegisterWindowClass(appInstance);
    if (!InitializeWindow(appInstance, showCommand))
        return FALSE;

    HWND mainWindow = FindWindow(windowClassName, windowTitle);
    if (!mainWindow)
        return FALSE;

    if (FAILED(InitializeDirect3D(mainWindow)))
        return FALSE;

    if (FAILED(InitializeGraphicsResources()))
    {
        ReleaseDirect3DResources();
        return FALSE;
    }

    MSG message = {};
    while (message.message != WM_QUIT)
    {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else
        {
            DrawScene();
        }
    }
    ReleaseDirect3DResources();
    return (int)message.wParam;
}

ATOM RegisterWindowClass(HINSTANCE appInstance)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = appInstance;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszClassName = windowClassName;
    return RegisterClassExW(&windowClass);
}

BOOL InitializeWindow(HINSTANCE appInstance, int showCommand)
{
    HWND mainWindow = CreateWindowW(windowClassName, windowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600,
        nullptr, nullptr, appInstance, nullptr);
    if (!mainWindow)
        return FALSE;
    ShowWindow(mainWindow, showCommand);
    UpdateWindow(mainWindow);
    return TRUE;
}

HRESULT InitializeDirect3D(HWND windowHandle)
{
    RECT clientRect;
    GetClientRect(windowHandle, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = windowHandle;
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

    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;

    D3D11_TEXTURE2D_DESC postProcessDesc = {};
    postProcessDesc.Width = width;
    postProcessDesc.Height = height;
    postProcessDesc.MipLevels = 1;
    postProcessDesc.ArraySize = 1;
    postProcessDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    postProcessDesc.SampleDesc.Count = 1;
    postProcessDesc.Usage = D3D11_USAGE_DEFAULT;
    postProcessDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    result = graphicsDevice->CreateTexture2D(&postProcessDesc, nullptr, &processedTexture);
    if (FAILED(result))
        return result;

    result = graphicsDevice->CreateRenderTargetView(processedTexture, nullptr, &processedTarget);
    if (FAILED(result))
        return result;

    result = graphicsDevice->CreateShaderResourceView(processedTexture, nullptr, &processedResource);
    if (FAILED(result))
        return result;

    return S_OK;
}

HRESULT InitializeGraphicsResources()
{
    HRESULT result = S_OK;
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* pixelShaderBlob = nullptr;

    result = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Vertex Shader Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainVertexShader);
    if (FAILED(result))
    {
        shaderBlob->Release();
        return result;
    }
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                        D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,           D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(float) * 6,           D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    result = graphicsDevice->CreateInputLayout(vertexLayout, 3, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &inputLayout);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Pixel Shader Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainPixelShader);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VertexData) * ARRAYSIZE(cubeVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = cubeVertices;
    result = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &vertexData);
    if (FAILED(result))
        return result;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    const int instanceCount = 21;
    bufferDesc.ByteWidth = sizeof(XMMATRIX) * instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    result = graphicsDevice->CreateBuffer(&bufferDesc, nullptr, &modelBuffer);
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC viewProjBufferDesc = {};
    viewProjBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    viewProjBufferDesc.ByteWidth = sizeof(XMMATRIX);
    viewProjBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    viewProjBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    result = graphicsDevice->CreateBuffer(&viewProjBufferDesc, nullptr, &viewProjectionBuffer);
    if (FAILED(result))
        return result;

    result = CreateDDSTextureFromFile(graphicsDevice, L"cube.dds", nullptr, &cubeTexture);
    if (FAILED(result))
        return result;

    result = CreateDDSTextureFromFile(graphicsDevice, L"cube_normal.dds", nullptr, &cubeNormalMap);
    if (FAILED(result))
        return result;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    result = graphicsDevice->CreateSamplerState(&samplerDesc, &textureSampler);
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"Skybox_VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Skybox VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &skyVertexShader);
    if (FAILED(result))
    {
        shaderBlob->Release();
        return result;
    }
    D3D11_INPUT_ELEMENT_DESC skyboxVertexLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    result = graphicsDevice->CreateInputLayout(skyboxVertexLayout, 1, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &skyInputLayout);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"Skybox_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Skybox PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &skyPixelShader);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(SkyVertex) * ARRAYSIZE(skyboxVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyboxVertices;
    result = graphicsDevice->CreateBuffer(&bufferDesc, &initData, &skyVertexData);
    if (FAILED(result))
        return result;

    viewProjBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    viewProjBufferDesc.ByteWidth = sizeof(XMMATRIX);
    viewProjBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    viewProjBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    result = graphicsDevice->CreateBuffer(&viewProjBufferDesc, nullptr, &skyViewProjBuffer);
    if (FAILED(result))
        return result;

    result = CreateDDSTextureFromFile(graphicsDevice, L"skybox.dds", nullptr, &skyTexture);
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"Transparent_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Transparent PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &transparentPixelShader);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC transparentBufferDesc = {};
    transparentBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    transparentBufferDesc.ByteWidth = sizeof(XMFLOAT4);
    transparentBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    transparentBufferDesc.CPUAccessFlags = 0;
    result = graphicsDevice->CreateBuffer(&transparentBufferDesc, nullptr, &transparentDataBuffer);
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufferDesc.ByteWidth = sizeof(LightParameters);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = 0;
    result = graphicsDevice->CreateBuffer(&lightBufferDesc, nullptr, &lightingDataBuffer);
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"Light_PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Light PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &lightEffectShader);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC lightColorDesc = {};
    lightColorDesc.Usage = D3D11_USAGE_DEFAULT;
    lightColorDesc.ByteWidth = sizeof(XMFLOAT4);
    lightColorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightColorDesc.CPUAccessFlags = 0;
    result = graphicsDevice->CreateBuffer(&lightColorDesc, nullptr, &lightColorData);
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC colorBuffer = {};
    colorBuffer.Usage = D3D11_USAGE_DEFAULT;
    colorBuffer.ByteWidth = sizeof(XMFLOAT4);
    colorBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    colorBuffer.CPUAccessFlags = 0;

    result = graphicsDevice->CreateBuffer(&colorBuffer, nullptr, &blueLightBuffer);
    if (FAILED(result)) return result;

    result = graphicsDevice->CreateBuffer(&colorBuffer, nullptr, &greenLightBuffer);
    if (FAILED(result)) return result;

    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = graphicsDevice->CreateBlendState(&blendStateDesc, &blendState);
    if (FAILED(result))
        return result;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    result = graphicsDevice->CreateDepthStencilState(&depthDesc, &depthStateTransparent);
    if (FAILED(result))
        return result;

    result = D3DCompileFromFile(L"transparent_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);
    if (FAILED(result))
    {
        if (errorBlob)
        {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Transparent VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return result;
    }
    result = graphicsDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &transparentVertexShader);
    shaderBlob->Release();
    if (FAILED(result))
        return result;

    ID3DBlob* postProcessVSBlob = nullptr;
    ID3DBlob* postProcessPSBlob = nullptr;
    ID3DBlob* postProcessErrorBlob = nullptr;

    result = D3DCompileFromFile(
        L"PostProcessVS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        0, 0,
        &postProcessVSBlob,
        &postProcessErrorBlob
    );
    if (FAILED(result))
    {
        if (postProcessErrorBlob)
        {
            MessageBoxA(nullptr, (char*)postProcessErrorBlob->GetBufferPointer(),
                "PostProcess VS Compile Error", MB_OK);
            postProcessErrorBlob->Release();
        }
        return result;
    }

    result = graphicsDevice->CreateVertexShader(
        postProcessVSBlob->GetBufferPointer(),
        postProcessVSBlob->GetBufferSize(),
        nullptr,
        &postProcessVS
    );
    if (FAILED(result))
    {
        postProcessVSBlob->Release();
        return result;
    }

    D3D11_INPUT_ELEMENT_DESC postProcessLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(float) * 4,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    result = graphicsDevice->CreateInputLayout(
        postProcessLayout,
        _countof(postProcessLayout),
        postProcessVSBlob->GetBufferPointer(),
        postProcessVSBlob->GetBufferSize(),
        &screenLayout
    );
    if (FAILED(result))
    {
        postProcessVSBlob->Release();
        return result;
    }

    postProcessVSBlob->Release();
    postProcessVSBlob = nullptr;

    result = D3DCompileFromFile(
        L"PostProcessPS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "ps_5_0",
        0, 0,
        &postProcessPSBlob,
        &postProcessErrorBlob
    );
    if (FAILED(result))
    {
        if (postProcessErrorBlob)
        {
            MessageBoxA(nullptr, (char*)postProcessErrorBlob->GetBufferPointer(),
                "PostProcess PS Compile Error", MB_OK);
            postProcessErrorBlob->Release();
        }
        return result;
    }

    result = graphicsDevice->CreatePixelShader(
        postProcessPSBlob->GetBufferPointer(),
        postProcessPSBlob->GetBufferSize(),
        nullptr,
        &postProcessPS
    );

    postProcessPSBlob->Release();
    postProcessPSBlob = nullptr;

    if (FAILED(result))
    {
        return result;
    }

    {
        D3D11_BUFFER_DESC screenQuadDesc = {};
        screenQuadDesc.Usage = D3D11_USAGE_DEFAULT;
        screenQuadDesc.ByteWidth = sizeof(ScreenVertex) * 3;
        screenQuadDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        screenQuadDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA quadInitData = {};
        quadInitData.pSysMem = fullscreenTriangle;

        result = graphicsDevice->CreateBuffer(&screenQuadDesc, &quadInitData, &screenQuadBuffer);
        if (FAILED(result))
        {
            return result;
        }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(FindWindow(windowClassName, windowTitle));

    ImGui_ImplDX11_Init(graphicsDevice, deviceContext);

    return S_OK;
}

void ReleaseDirect3DResources()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (deviceContext)
        deviceContext->ClearState();

    if (vertexData) vertexData->Release();
    if (inputLayout) inputLayout->Release();
    if (mainVertexShader) mainVertexShader->Release();
    if (mainPixelShader) mainPixelShader->Release();
    if (modelBuffer) modelBuffer->Release();
    if (viewProjectionBuffer) viewProjectionBuffer->Release();
    if (cubeTexture) cubeTexture->Release();

    if (cubeNormalMap) cubeNormalMap->Release();
    if (textureSampler) textureSampler->Release();
    if (renderTarget) renderTarget->Release();
    if (depthBuffer) depthBuffer->Release();
    if (swapChain) swapChain->Release();
    if (deviceContext) deviceContext->Release();
    if (graphicsDevice) graphicsDevice->Release();

    if (skyVertexData) skyVertexData->Release();
    if (skyInputLayout) skyInputLayout->Release();
    if (skyVertexShader) skyVertexShader->Release();
    if (skyPixelShader) skyPixelShader->Release();
    if (skyViewProjBuffer) skyViewProjBuffer->Release();
    if (skyTexture) skyTexture->Release();

    if (transparentPixelShader) transparentPixelShader->Release();
    if (transparentDataBuffer) transparentDataBuffer->Release();
    if (blendState) blendState->Release();
    if (depthStateTransparent) depthStateTransparent->Release();

    if (lightingDataBuffer) lightingDataBuffer->Release();
    if (lightEffectShader) lightEffectShader->Release();
    if (lightColorData) lightColorData->Release();
    if (blueLightBuffer) blueLightBuffer->Release();
    if (greenLightBuffer) greenLightBuffer->Release();
    if (transparentVertexShader) transparentVertexShader->Release();

    if (processedTexture) processedTexture->Release();
    if (processedTarget) processedTarget->Release();
    if (processedResource) processedResource->Release();
    if (postProcessVS) postProcessVS->Release();
    if (postProcessPS) postProcessPS->Release();

    if (screenQuadBuffer) screenQuadBuffer->Release();
    if (screenLayout) screenLayout->Release();
}

void DrawObjects(bool renderToBackBuffer)
{
    if (renderToBackBuffer)
    {
        deviceContext->OMSetRenderTargets(1, &renderTarget, depthBuffer);
        float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
        deviceContext->ClearRenderTargetView(renderTarget, clearColor);
        deviceContext->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    RECT clientRect;
    GetClientRect(FindWindow(windowClassName, windowTitle), &clientRect);
    float aspectRatio = static_cast<float>(clientRect.right - clientRect.left) / (clientRect.bottom - clientRect.top);
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

    float cameraDistance = 6.0f;
    float cameraX = cameraDistance * sinf(cameraHorizontalAngle) * cosf(cameraVerticalAngle);
    float cameraY = cameraDistance * sinf(cameraVerticalAngle);
    float cameraZ = cameraDistance * cosf(cameraHorizontalAngle) * cosf(cameraVerticalAngle);
    XMVECTOR cameraPosition = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    XMVECTOR lookAtPoint = XMVectorZero();
    XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(cameraPosition, lookAtPoint, upVector);

    XMMATRIX skyboxViewMatrix = viewMatrix;
    skyboxViewMatrix.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX skyboxViewProjection = XMMatrixTranspose(skyboxViewMatrix * projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    if (SUCCEEDED(deviceContext->Map(skyViewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &skyboxViewProjection, sizeof(XMMATRIX));
        deviceContext->Unmap(skyViewProjBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* skyboxDepthState = nullptr;
    graphicsDevice->CreateDepthStencilState(&depthStencilDesc, &skyboxDepthState);
    deviceContext->OMSetDepthStencilState(skyboxDepthState, 0);

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_FRONT;
    rasterizerDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* skyboxRasterizerState = nullptr;
    if (SUCCEEDED(graphicsDevice->CreateRasterizerState(&rasterizerDesc, &skyboxRasterizerState)))
    {
        deviceContext->RSSetState(skyboxRasterizerState);
    }

    UINT vertexStride = sizeof(SkyVertex);
    UINT vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &skyVertexData, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(skyInputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(skyVertexShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &skyViewProjBuffer);
    deviceContext->PSSetShader(skyPixelShader, nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, &skyTexture);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);
    deviceContext->Draw(ARRAYSIZE(skyboxVertices), 0);

    skyboxDepthState->Release();
    if (skyboxRasterizerState)
    {
        skyboxRasterizerState->Release();
        deviceContext->RSSetState(nullptr);
    }
    deviceContext->OMSetDepthStencilState(nullptr, 0);

    cubeRotationAngle += 0.01f;

    XMMATRIX cubeViewProjection = XMMatrixTranspose(viewMatrix * projectionMatrix);
    if (SUCCEEDED(deviceContext->Map(viewProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
    {
        memcpy(mappedData.pData, &cubeViewProjection, sizeof(XMMATRIX));
        deviceContext->Unmap(viewProjectionBuffer, 0);
    }

    const int nearObjectsCount = 11; 
    const int farObjectsCount = 10;
    const int totalObjectsCount = nearObjectsCount + farObjectsCount;
    totalObjects = totalObjectsCount;
    
    XMMATRIX allObjectMatrices[totalObjectsCount];

    allObjectMatrices[0] = XMMatrixRotationY(cubeRotationAngle);
    float nearRadius = 3.0f;
    float nearAngleStep = XM_2PI / 10.0f;
    for (int i = 1; i < nearObjectsCount; i++)
    {
        float angleOffset = (i - 1) * nearAngleStep;
        float currentAngle = cubeRotationAngle + angleOffset;
        XMMATRIX positionMatrix = XMMatrixTranslation(nearRadius * cosf(currentAngle), 0.0f, nearRadius * sinf(currentAngle));
        XMMATRIX rotationMatrix = XMMatrixRotationY(cubeRotationAngle);
        allObjectMatrices[i] = positionMatrix * rotationMatrix;
    }

    float farRadius = 40.0f;
    float farAngleStep = XM_2PI / farObjectsCount;
    for (int i = nearObjectsCount; i < totalObjectsCount; i++)
    {
        float angleOffset = (i - nearObjectsCount) * farAngleStep;
        float currentAngle = cubeRotationAngle + angleOffset;
        XMMATRIX positionMatrix = XMMatrixTranslation(farRadius * cosf(currentAngle), 0.0f, farRadius * sinf(currentAngle));
        XMMATRIX rotationMatrix = XMMatrixRotationY(cubeRotationAngle);
        allObjectMatrices[i] = positionMatrix * rotationMatrix;
    }

    XMMATRIX viewProjMatrix = viewMatrix * projectionMatrix;
    FrustumPlane frustumPlanes[6];
    GetFrustumPlanes(viewProjMatrix, frustumPlanes);

    for (int i = 0; i < 6; i++)
    {
        float length = sqrtf(frustumPlanes[i].x * frustumPlanes[i].x +
            frustumPlanes[i].y * frustumPlanes[i].y +
            frustumPlanes[i].z * frustumPlanes[i].z);
        frustumPlanes[i].x /= length;
        frustumPlanes[i].y /= length;
        frustumPlanes[i].z /= length;
        frustumPlanes[i].w /= length;
    }

    XMMATRIX visibleObjectsMatrices[totalObjectsCount];
    int visibleCount = 0;
    if (enableViewCulling)
    {
        for (int i = 0; i < totalObjectsCount; i++)
        {
            XMVECTOR objectCenter = XMVectorSet(allObjectMatrices[i].r[3].m128_f32[0],
                allObjectMatrices[i].r[3].m128_f32[1],
                allObjectMatrices[i].r[3].m128_f32[2],
                1.0f);
            if (CheckSphereVisibility(frustumPlanes, objectCenter, 1.0f))
            {
                visibleObjectsMatrices[visibleCount++] = allObjectMatrices[i];
            }
        }
    }
    else
    {
        memcpy(visibleObjectsMatrices, allObjectMatrices, sizeof(allObjectMatrices));
        visibleCount = totalObjectsCount;
    }
    visibleObjects = visibleCount;
    XMMATRIX transposedMatrices[totalObjectsCount];
    for (int i = 0; i < visibleCount; i++)
    {
        transposedMatrices[i] = XMMatrixTranspose(visibleObjectsMatrices[i]);
    }
    deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, transposedMatrices, 0, 0);

    vertexStride = sizeof(VertexData);
    vertexOffset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexData, &vertexStride, &vertexOffset);
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(mainVertexShader, nullptr, 0);
    deviceContext->PSSetShader(mainPixelShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &modelBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &viewProjectionBuffer);
    deviceContext->PSSetConstantBuffers(0, 1, &lightingDataBuffer);

    ID3D11ShaderResourceView* cubeTextures[2] = { cubeTexture, cubeNormalMap };
    deviceContext->PSSetShaderResources(0, 2, cubeTextures);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);

    deviceContext->DrawInstanced(ARRAYSIZE(cubeVertices), visibleCount, 0, 0);

    float blendFactors[4] = { 0, 0, 0, 0 };
    deviceContext->OMSetBlendState(blendState, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(depthStateTransparent, 0);
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

    float blueDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(bluePosition, cameraPosition)));
    float greenDistance = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(greenPosition, cameraPosition)));

    deviceContext->PSSetShader(transparentPixelShader, nullptr, 0);
    deviceContext->VSSetShader(transparentVertexShader, nullptr, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &lightingDataBuffer);

    if (blueDistance >= greenDistance)
    {
        XMMATRIX transposedBlueModel = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &transposedBlueModel, 0, 0);
        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(blueLightBuffer, 0, nullptr, &blueColor, 0, 0);
        deviceContext->PSSetConstantBuffers(1, 1, &blueLightBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX transposedGreenModel = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &transposedGreenModel, 0, 0);
        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(greenLightBuffer, 0, nullptr, &greenColor, 0, 0);
        deviceContext->PSSetConstantBuffers(1, 1, &greenLightBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }
    else
    {
        XMMATRIX transposedGreenModel = XMMatrixTranspose(greenModel);
        deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &transposedGreenModel, 0, 0);
        XMFLOAT4 greenColor(0.0f, 1.0f, 0.0f, 0.5f);
        deviceContext->UpdateSubresource(greenLightBuffer, 0, nullptr, &greenColor, 0, 0);
        deviceContext->PSSetConstantBuffers(1, 1, &greenLightBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

        XMMATRIX transposedBlueModel = XMMatrixTranspose(blueModel);
        deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &transposedBlueModel, 0, 0);
        XMFLOAT4 blueColor(0.0f, 0.0f, 1.0f, 0.5f);
        deviceContext->UpdateSubresource(blueLightBuffer, 0, nullptr, &blueColor, 0, 0);
        deviceContext->PSSetConstantBuffers(1, 1, &blueLightBuffer);
        deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
    }

    deviceContext->OMSetBlendState(nullptr, blendFactors, 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(nullptr, 0);

    LightParameters lightParams;
    lightParams.lightPosition0 = XMFLOAT3(1.0f, 0.0f, 0.0f);
    lightParams.lightColor0 = XMFLOAT3(3.0f, 3.0f, 0.0f);
    lightParams.lightPosition1 = XMFLOAT3(-1.0f, 0.0f, 0.0f);
    lightParams.lightColor1 = XMFLOAT3(3.0f, 3.0f, 3.0f);
    lightParams.ambientLight = XMFLOAT3(0.0f, 0.0f, 0.0f);
    deviceContext->UpdateSubresource(lightingDataBuffer, 0, nullptr, &lightParams, 0, 0);

    deviceContext->PSSetShader(lightEffectShader, nullptr, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &lightColorData);

    XMMATRIX lightScaleMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);

    XMMATRIX lightPosition0 = XMMatrixTranslation(1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel0 = XMMatrixTranspose(lightScaleMatrix * lightPosition0);
    deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &lightModel0, 0, 0);
    XMFLOAT4 lightColor0(1.0f, 1.0f, 0.0f, 1.0f);
    deviceContext->UpdateSubresource(lightColorData, 0, nullptr, &lightColor0, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);

    XMMATRIX lightPosition1 = XMMatrixTranslation(-1.0f, 0.0f, 0.0f);
    XMMATRIX lightModel1 = XMMatrixTranspose(lightScaleMatrix * lightPosition1);
    deviceContext->UpdateSubresource(modelBuffer, 0, nullptr, &lightModel1, 0, 0);
    XMFLOAT4 lightColor1(1.0f, 1.0f, 1.0f, 1.0f);
    deviceContext->UpdateSubresource(lightColorData, 0, nullptr, &lightColor1, 0, 0);
    deviceContext->Draw(ARRAYSIZE(cubeVertices), 0);
}

void DrawScene()
{
    float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

    if (enableImageFilter)
    {
        deviceContext->OMSetRenderTargets(1, &processedTarget, depthBuffer);
        deviceContext->ClearRenderTargetView(processedTarget, clearColor);
        deviceContext->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
        DrawObjects(false);
    }
    else
    {
        deviceContext->OMSetRenderTargets(1, &renderTarget, depthBuffer);
        deviceContext->ClearRenderTargetView(renderTarget, clearColor);
        deviceContext->ClearDepthStencilView(depthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
        DrawObjects(true);
    }
    if (enableImageFilter)
    {
        deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
        deviceContext->OMSetDepthStencilState(nullptr, 0);

        UINT vertexStride = sizeof(ScreenVertex);
        UINT vertexOffset = 0;
        deviceContext->IASetVertexBuffers(0, 1, &screenQuadBuffer, &vertexStride, &vertexOffset);
        deviceContext->IASetInputLayout(screenLayout);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        deviceContext->VSSetShader(postProcessVS, nullptr, 0);
        deviceContext->PSSetShader(postProcessPS, nullptr, 0);

        deviceContext->PSSetShaderResources(0, 1, &processedResource);
        deviceContext->PSSetSamplers(0, 1, &textureSampler);

        deviceContext->Draw(3, 0);

        ID3D11ShaderResourceView* nullResources[1] = { nullptr };
        deviceContext->PSSetShaderResources(0, 1, nullResources);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(60, 40), ImGuiCond_FirstUseEver);

    ImGui::Begin("Options");
    ImGui::Checkbox("Grayscale Filter", &enableImageFilter);
    ImGui::Checkbox("Frustum Culling", &enableViewCulling);
    ImGui::Text("Total cubes: %d", totalObjects);
    ImGui::Text("Visible cubes: %d", visibleObjects);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        isMouseDragging = true;
        previousMousePosition.x = LOWORD(lParam);
        previousMousePosition.y = HIWORD(lParam);
        SetCapture(windowHandle);
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
            int deltaX = currentX - previousMousePosition.x;
            int deltaY = currentY - previousMousePosition.y;
            cameraHorizontalAngle += deltaX * 0.005f;
            cameraVerticalAngle += deltaY * 0.005f;
            if (cameraVerticalAngle > XM_PIDIV2 - 0.01f)
                cameraVerticalAngle = XM_PIDIV2 - 0.01f;
            if (cameraVerticalAngle < -XM_PIDIV2 + 0.01f)
                cameraVerticalAngle = -XM_PIDIV2 + 0.01f;
            previousMousePosition.x = currentX;
            previousMousePosition.y = currentY;
        }
        break;
    case WM_KEYDOWN:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(windowHandle, message, wParam, lParam);
    }
    return 0;
}