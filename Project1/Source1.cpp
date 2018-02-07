#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <wrl\client.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;


ComPtr<IDXGIDevice1> CreateDXGIDevice()
{
    ComPtr<ID3D11Device> device;
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, // Direct2Dではこのフラグが必要
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device,
        nullptr,
        nullptr
    );

    ComPtr<IDXGIDevice1> dxgiDevice;
    device.As(&dxgiDevice);
    dxgiDevice->SetMaximumFrameLatency(1); // レイテンシを最小にしておく

    return dxgiDevice;
}


ComPtr<IDXGISwapChain1> CreateSwapChain(IDXGIDevice1* device, HWND hwnd)
{
    ComPtr<IDXGIAdapter> adapter;
    device->GetAdapter(&adapter);

    ComPtr<IDXGIFactory2> factory;
    adapter->GetParent(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 desc = { 0 };
    desc.Width = 0; // 自動でWindowのサイズが設定される
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = 0;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        device,
        hwnd,
        &desc,
        nullptr,
        nullptr,
        &swapChain
    );

    return swapChain;
}


ComPtr<ID2D1Bitmap1> CreateBackBufferBitmap(ID2D1DeviceContext* context, IDXGISwapChain1* swapChain)
{
    ComPtr<IDXGISurface> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    auto prop = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );
    ComPtr<ID2D1Bitmap1> bitmap;
    context->CreateBitmapFromDxgiSurface(backBuffer.Get(), &prop, &bitmap);

    return bitmap;
}


ComPtr<ID2D1DeviceContext> CreateD2DContext(IDXGIDevice1* dxgiDevice)
{
    ComPtr<ID2D1Factory1> factory;
    D2D1CreateFactory<ID2D1Factory1>(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

    ComPtr<ID2D1Device> device;
    factory->CreateDevice(dxgiDevice, &device);

    ComPtr<ID2D1DeviceContext> context;
    device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);

    return context;
}


ComPtr<ID2D1HwndRenderTarget> CreateRenderTarget(HWND hwnd)
{
    ComPtr<ID2D1Factory> factory;
    D2D1CreateFactory<ID2D1Factory>(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

    ComPtr<ID2D1HwndRenderTarget> renderTarget;
    factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(), 
        D2D1::HwndRenderTargetProperties(hwnd), 
        &renderTarget
    );

    return renderTarget;
}


void Draw(ID2D1HwndRenderTarget* renderTarget)
{
    renderTarget->BeginDraw();

    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightYellow));

    D2D1_SIZE_F size = renderTarget->GetSize();
    float rx = size.width / 2;
    float ry = size.height / 2;
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(rx, ry), rx, ry);
    ComPtr<ID2D1SolidColorBrush> brush;
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &brush);
    renderTarget->FillEllipse(ellipse, brush.Get());

    renderTarget->EndDraw();
}


void Draw(ID2D1DeviceContext* context)
{
    context->BeginDraw();

    context->Clear(D2D1::ColorF(D2D1::ColorF::LightYellow));

    D2D1_SIZE_F size = context->GetSize();
    float rx = size.width / 2;
    float ry = size.height / 2;
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(rx, ry), rx, ry);
    ComPtr<ID2D1SolidColorBrush> brush;
    context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &brush);
    context->FillEllipse(ellipse, brush.Get());

    context->EndDraw();
}


ComPtr<ID2D1DeviceContext> g_context;
ComPtr<IDXGISwapChain1> g_swapChain;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
        {
            Draw(g_context.Get());
            DXGI_PRESENT_PARAMETERS params = { 0 };
            g_swapChain->Present1(1, 0, &params);
        }
        return 0;
    case WM_SIZE:
        g_context->SetTarget(nullptr);
        g_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0); // サイズ0で、自動でWindowのサイズが設定される
        g_context->SetTarget(CreateBackBufferBitmap(g_context.Get(), g_swapChain.Get()).Get());
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


HWND CreateHWND(WNDPROC wndProc)
{
    WNDCLASSEX wndclass;
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndclass.lpfnWndProc = wndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = GetModuleHandle(NULL);
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "Main Window";
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wndclass);

    HWND hwnd = CreateWindow(
        "Main Window",
        "Main Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        wndclass.hInstance,
        NULL
    );

    return hwnd;
}


void Run(HWND hwnd)
{
    ShowWindow(hwnd, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


int main()
{
	HWND hwnd = CreateHWND(WndProc);

    auto dxgiDevice = CreateDXGIDevice();
    g_context = CreateD2DContext(dxgiDevice.Get());
    g_swapChain = CreateSwapChain(dxgiDevice.Get(), hwnd);
    g_context->SetTarget(CreateBackBufferBitmap(g_context.Get(), g_swapChain.Get()).Get());

    Run(hwnd);

    return 0;
}

