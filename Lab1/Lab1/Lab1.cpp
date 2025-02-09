#include "framework.h"
#include "Lab1.h"
#include "Render.h"
#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

#define MAX_LOADSTRING 100

WCHAR szTitle[MAX_LOADSTRING] = L"Lab1 Graphics App";
WCHAR szWindowClass[MAX_LOADSTRING] = L"LAB1";

ATOM                RegisterWindowClass(HINSTANCE hInstance);
BOOL                InitializeAppInstance(HINSTANCE, int);
LRESULT CALLBACK    WindowProc(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<Render> g_Render;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    RegisterWindowClass(hInstance);

    if (!InitializeAppInstance(hInstance, nCmdShow))
    {
        OutputDebugString(_T("Initialization failed.\n"));
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB1));

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
            if (g_Render && !IsIconic(g_Render->GetWindowHandle()))
            {
                g_Render->StartFrame();
            }
        }
    }

    if (g_Render)
    {
        g_Render->Shutdown();
        g_Render.reset();
    }

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
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

BOOL InitializeAppInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                                100, 100, 800, 600,
                                nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugString(_T("Failed to create window.\n"));
        return FALSE;
    }

    g_Render = std::make_unique<Render>(hWnd);
    if (FAILED(g_Render->Initialize()))
    {
        OutputDebugString(_T("Failed to initialize Render.\n"));
        g_Render.reset();
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (g_Render && wParam != SIZE_MINIMIZED)
        {
            g_Render->ResizeViewport();
        }
        return 0;

    case WM_MOUSEMOVE:
        if (g_Render)
        {
            g_Render->UpdateMousePosition(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_PAINT:
        if (g_Render)
        {
            g_Render->StartFrame();
        }
        ValidateRect(hWnd, nullptr);
        break;

    case WM_DESTROY:
        if (g_Render)
        {
            g_Render->Shutdown();
            g_Render.reset();
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}