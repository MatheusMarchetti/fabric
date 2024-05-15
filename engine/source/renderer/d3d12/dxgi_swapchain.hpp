#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "platform/platform.hpp"
#include "renderer/resources.hpp"

namespace fabric {
    class d3d12_command_list;
}

namespace fabric::dxgi_swapchain {
    void create(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* graphicsQueue, platform::window* window);
    void destroy();

    void begin_frame(const d3d12_command_list& cmdList);
    void end_frame(const d3d12_command_list& cmdList);
    b8 present(b8 vSync);

    void resize(u16 width, u16 height);

    // TODO: Temporary
    handle<texture> get_current_image();
}