#include "renderer/d3d12/dxgi_swapchain.hpp"
#include "renderer/d3d12/d3d12_backend.hpp"
#include "renderer/resources.hpp"

#include "core/logger.hpp"

using namespace fabric;

namespace {
    struct win32_state {
        HINSTANCE instance;
        HWND handle;
    };

    IDXGISwapChain4* swapchain;
    handle<texture> swapchain_images[swapchain_image_count];

    u8 current_image_index;
}  // namespace

void dxgi_swapchain::create(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* graphicsQueue, platform::window* window) {
    HWND hwnd;
    win32_state* state = (win32_state*)window->internal_state;
    hwnd = state->handle;

    IDXGISwapChain1* swapchain1;
    BOOL allowTearing = 0;

    HRCheck(dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));

    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
        .Width = window->width,
        .Height = window->height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0},
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = swapchain_image_count,
        .Scaling = DXGI_SCALING_NONE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = static_cast<UINT>(allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)};

    HRCheck(dxgiFactory->CreateSwapChainForHwnd(graphicsQueue, hwnd, &swapchain_desc, nullptr, nullptr, &swapchain1));
    HRCheck(swapchain1->QueryInterface(&swapchain));
    swapchain1->Release();

    dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
    };

    for (u8 image_index = 0; image_index < swapchain_image_count; image_index++) {
        ID3D12Resource2* buffer;
        HRCheck(swapchain->GetBuffer(image_index, IID_PPV_ARGS(&buffer)));
        // TODO: Name the swapchain images with their indices

        swapchain_images[image_index] = backend::create_render_target(buffer, rtv_desc);
    }

    current_image_index = swapchain->GetCurrentBackBufferIndex();
}

void dxgi_swapchain::destroy() {
    swapchain->Release();
}

