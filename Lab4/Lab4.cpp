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

ID3D11Device* d3dDevice = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTarget = nullptr;
ID3D11DepthStencilView* depthBuffer = nullptr;

ID3D11VertexShader* mainVS = nullptr;
ID3D11PixelShader* mainPS = nullptr;
ID3D11InputLayout* inputLayout = nullptr;
ID3D11Buffer* vertexData = nullptr;
ID3D11Buffer* transformBuffer = nullptr;
ID3D11Buffer* viewProjBuffer = nullptr;

ID3D11ShaderResourceView* textureView = nullptr;
ID3D11SamplerState* textureSampler = nullptr;

ID3D11VertexShader* backgroundVS = nullptr;
ID3D11PixelShader* backgroundPS = nullptr;
ID3D11InputLayout* backgroundLayout = nullptr;
ID3D11Buffer* backgroundVertices = nullptr;
ID3D11Buffer* backgroundVPBuffer = nullptr;
ID3D11ShaderResourceView* backgroundTexture = nullptr;

float rotationAngle = 0.0f;
float viewAngle = 0.0f;
bool   isDragging = false;
POINT  lastCursorPos = { 0, 0 };
float  horizontalAngle = 0.0f;
float  verticalAngle = 0.0f;

struct VertexData
{
    float posX, posY, posZ;
    float texU, texV;
};

struct BackgroundVertex
{
    float x, y, z;
};

VertexData cubeGeometry[] =
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

BackgroundVertex skyGeometry[] =
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

    UINT createFlags = 0;
#if defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createFlags, featureLevels, 1,
        D3D11_SDK_VERSION, &swapDesc, &swapChain,
        &d3dDevice, &featureLevel, &deviceContext);
    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;
    hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    backBuffer->Release();
    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* depthTexture = nullptr;
    hr = d3dDevice->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
    if (FAILED(hr))
        return hr;
    hr = d3dDevice->CreateDepthStencilView(depthTexture, nullptr, &depthBuffer);
    depthTexture->Release();
    if (FAILED(hr))
        return hr;

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
    hr = d3dDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainVS);
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
    hr = d3dDevice->CreateInputLayout(layout, 2, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &inputLayout);
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
    hr = d3dDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &mainPS);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VertexData) * ARRAYSIZE(cubeGeometry);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = cubeGeometry;
    hr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexData);
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(XMMATRIX);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    hr = d3dDevice->CreateBuffer(&bufferDesc, nullptr, &transformBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC vpDesc = {};
    vpDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpDesc.ByteWidth = sizeof(XMMATRIX);
    vpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = d3dDevice->CreateBuffer(&vpDesc, nullptr, &viewProjBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(d3dDevice, L"cube.dds", nullptr, &textureView);
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
    hr = d3dDevice->CreateSamplerState(&samplerDesc, &textureSampler);
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
    hr = d3dDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &backgroundVS);
    if (FAILED(hr))
    {
        shaderBlob->Release();
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = d3dDevice->CreateInputLayout(skyboxLayout, 1, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &backgroundLayout);
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
    hr = d3dDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        nullptr, &backgroundPS);
    shaderBlob->Release();
    if (FAILED(hr))
        return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(BackgroundVertex) * ARRAYSIZE(skyGeometry);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyGeometry;
    hr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &backgroundVertices);
    if (FAILED(hr))
        return hr;

    vpDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpDesc.ByteWidth = sizeof(XMMATRIX);
    vpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = d3dDevice->CreateBuffer(&vpDesc, nullptr, &backgroundVPBuffer);
    if (FAILED(hr))
        return hr;

    hr = CreateDDSTextureFromFile(d3dDevice, L"skybox.dds", nullptr, &backgroundTexture);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void ReleaseResources()
{
    if (deviceContext)
        deviceContext->ClearState();

    if (vertexData) vertexData->Release();
    if (inputLayout) inputLayout->Release();
    if (mainVS) mainVS->Release();
    if (mainPS) mainPS->Release();
    if (transformBuffer) transformBuffer->Release();
    if (viewProjBuffer) viewProjBuffer->Release();
    if (textureView) textureView->Release();
    if (textureSampler) textureSampler->Release();
    if (renderTarget) renderTarget->Release();
    if (depthBuffer) depthBuffer->Release();
    if (swapChain) swapChain->Release();
    if (deviceContext) deviceContext->Release();
    if (d3dDevice) d3dDevice->Release();

    if (backgroundVertices) backgroundVertices->Release();
    if (backgroundLayout) backgroundLayout->Release();
    if (backgroundVS) backgroundVS->Release();
    if (backgroundPS) backgroundPS->Release();
    if (backgroundVPBuffer) backgroundVPBuffer->Release();
    if (backgroundTexture) backgroundTexture->Release();
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

    float distance = 5.0f;
    float cameraX = distance * sinf(horizontalAngle) * cosf(verticalAngle);
    float cameraY = distance * sinf(verticalAngle);
    float cameraZ = distance * cosf(horizontalAngle) * cosf(verticalAngle);
    XMVECTOR eyePosition = XMVectorSet(cameraX, cameraY, cameraZ, 0.0f);
    XMVECTOR lookAt = XMVectorZero();
    XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, lookAt, upDirection);

    XMMATRIX skyboxView = viewMatrix;
    skyboxView.r[3] = XMVectorSet(0, 0, 0, 1);
    XMMATRIX skyboxVP = XMMatrixTranspose(skyboxView * projection);

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(deviceContext->Map(backgroundVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        memcpy(mapped.pData, &skyboxVP, sizeof(XMMATRIX));
        deviceContext->Unmap(backgroundVPBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* skyboxDepthState = nullptr;
    d3dDevice->CreateDepthStencilState(&depthDesc, &skyboxDepthState);
    deviceContext->OMSetDepthStencilState(skyboxDepthState, 0);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_FRONT;
    rasterDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* skyboxRasterState = nullptr;
    if (SUCCEEDED(d3dDevice->CreateRasterizerState(&rasterDesc, &skyboxRasterState)))
    {
        deviceContext->RSSetState(skyboxRasterState);
    }

    UINT stride = sizeof(BackgroundVertex);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &backgroundVertices, &stride, &offset);
    deviceContext->IASetInputLayout(backgroundLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(backgroundVS, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &backgroundVPBuffer);
    deviceContext->PSSetShader(backgroundPS, nullptr, 0);
    deviceContext->PSSetShaderResources(0, 1, &backgroundTexture);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);
    deviceContext->Draw(ARRAYSIZE(skyGeometry), 0);

    skyboxDepthState->Release();
    if (skyboxRasterState)
    {
        skyboxRasterState->Release();
        deviceContext->RSSetState(nullptr);
    }

    deviceContext->OMSetRenderTargets(1, &renderTarget, depthBuffer);

    rotationAngle += 0.01f;
    XMMATRIX modelMatrix = XMMatrixRotationY(rotationAngle);
    XMMATRIX modelTransposed = XMMatrixTranspose(modelMatrix);
    deviceContext->UpdateSubresource(transformBuffer, 0, nullptr, &modelTransposed, 0, 0);

    XMMATRIX cubeVP = XMMatrixTranspose(viewMatrix * projection);
    if (SUCCEEDED(deviceContext->Map(viewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        memcpy(mapped.pData, &cubeVP, sizeof(XMMATRIX));
        deviceContext->Unmap(viewProjBuffer, 0);
    }

    stride = sizeof(VertexData);
    offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexData, &stride, &offset);
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(mainVS, nullptr, 0);
    deviceContext->PSSetShader(mainPS, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &transformBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &viewProjBuffer);
    deviceContext->PSSetShaderResources(0, 1, &textureView);
    deviceContext->PSSetSamplers(0, 1, &textureSampler);
    deviceContext->Draw(ARRAYSIZE(cubeGeometry), 0);

    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        isDragging = true;
        lastCursorPos.x = LOWORD(lParam);
        lastCursorPos.y = HIWORD(lParam);
        SetCapture(hWnd);
        break;
    case WM_LBUTTONUP:
        isDragging = false;
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        if (isDragging)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int dx = x - lastCursorPos.x;
            int dy = y - lastCursorPos.y;
            horizontalAngle += dx * 0.005f;
            verticalAngle += dy * 0.005f;
            if (verticalAngle > XM_PIDIV2 - 0.01f)
                verticalAngle = XM_PIDIV2 - 0.01f;
            if (verticalAngle < -XM_PIDIV2 + 0.01f)
                verticalAngle = -XM_PIDIV2 + 0.01f;
            lastCursorPos.x = x;
            lastCursorPos.y = y;
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