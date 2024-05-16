#pragma once

#include "renderer/d3d12/d3d12_common.hpp"
#include "platform/platform.hpp"

namespace fabric {
    class d3d12_command_list;
    class d3d12_resource;
}

namespace fabric::dxgi_swapchain {
    void create(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* graphicsQueue, platform::window* window);
    void destroy();

    void end_frame(const d3d12_command_list& cmdList, d3d12_resource& finalColor);
    b8 present(b8 vSync);

    void resize(u16 width, u16 height);
}