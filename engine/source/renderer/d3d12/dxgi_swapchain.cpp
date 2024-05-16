#include "renderer/d3d12/dxgi_swapchain.hpp"
#include "renderer/d3d12/d3d12_backend.hpp"
#include "renderer/d3d12/d3d12_command_list.hpp"
#include "renderer/d3d12/d3d12_resource.hpp"
#include "core/logger.hpp"

using namespace fabric;

namespace {
    struct win32_state {
        HINSTANCE instance;
        HWND handle;
    };

    IDXGISwapChain4* swapchain;
    d3d12_resource swapchain_resources[swapchain_image_count];
    descriptor_allocation swapchain_image_views[swapchain_image_count];

    u8 current_image_index;
    BOOL allowTearing = 0;
}  // namespace

void dxgi_swapchain::create(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* graphicsQueue, platform::window* window) {
    HWND hwnd;
    win32_state* state = (win32_state*)window->internal_state;
    hwnd = state->handle;

    IDXGISwapChain1* swapchain1;
    
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
        .Format = swapchain_desc.Format,
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D};

    for (u8 image_index = 0; image_index < swapchain_image_count; image_index++) {
        ID3D12Resource2* buffer;
        HRCheck(swapchain->GetBuffer(image_index, IID_PPV_ARGS(&buffer)));
        // TODO: Name the swapchain images with their indices

        swapchain_resources[image_index] = d3d12_resource(buffer, D3D12_RESOURCE_STATE_PRESENT);
        swapchain_image_views[image_index] = backend::create_render_target_view(buffer, rtv_desc);
    }

    current_image_index = swapchain->GetCurrentBackBufferIndex();
}

void dxgi_swapchain::destroy() {
    for(u32 i = 0; i < swapchain_image_count; i++) {
        swapchain_resources[i].release();
    }
    
    swapchain->Release();
}

void dxgi_swapchain::end_frame(const d3d12_command_list& cmdList, d3d12_resource& finalColor) {
    d3d12_resource* resources[] = {&swapchain_resources[current_image_index], &finalColor };
    const D3D12_RESOURCE_STATES after_states[] = {D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE};
    cmdList.transition_barriers(resources, after_states, _countof(resources));
    cmdList.copy_resource(finalColor, swapchain_resources[current_image_index]);
    cmdList.transition_barrier(&swapchain_resources[current_image_index], D3D12_RESOURCE_STATE_PRESENT);
}

b8 dxgi_swapchain::present(b8 vSync) {
    HRESULT result = swapchain->Present(vSync, allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
    current_image_index = swapchain->GetCurrentBackBufferIndex();

    return result == S_OK;
}

void dxgi_swapchain::resize(u16 width, u16 height) {
    for(u32 i = 0; i < swapchain_image_count; i++) {
        swapchain_resources[i].release();
    }
    DXGI_SWAP_CHAIN_DESC1 desc;
    HRCheck(swapchain->GetDesc1(&desc));
    HRCheck(swapchain->ResizeBuffers(swapchain_image_count, width, height, desc.Format, desc.Flags));

    current_image_index = swapchain->GetCurrentBackBufferIndex();

        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {
        .Format = desc.Format,
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D};

    for(u32 image_index = 0; image_index < swapchain_image_count; image_index++) {
        ID3D12Resource2* buffer;
        HRCheck(swapchain->GetBuffer(image_index,IID_PPV_ARGS(&buffer)));

        swapchain_resources[image_index] = d3d12_resource(buffer, D3D12_RESOURCE_STATE_PRESENT);
        swapchain_image_views[image_index] = backend::create_render_target_view(buffer, rtv_desc, swapchain_image_views[image_index].offset);
    }
}
