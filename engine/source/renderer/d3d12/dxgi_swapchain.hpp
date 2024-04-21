#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "platform/platform.hpp"

namespace fabric::dxgi_swapchain {
    void create(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* graphicsQueue, platform::window* window);
    void destroy();
}