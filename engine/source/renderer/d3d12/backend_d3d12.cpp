#include "renderer/d3d12/backend_d3d12.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "renderer/d3d12/d3d12_device.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/dxgi_swapchain.hpp"
#include "core/engine.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "platform/platform.hpp"

using namespace fabric;

namespace {
    u16 current_width;
    u16 current_height;

    u8 frame_index;
    u64 graphics_fence_values[frames_in_flight];
    u64 compute_fence_values[frames_in_flight];
    u64 transfer_fence_values[frames_in_flight];

    d3d12_command_queue graphics_queue;
    d3d12_command_queue compute_queue;
    d3d12_command_queue transfer_queue;
}  // namespace

b8 d3d12_initialize(void* state) {
    platform::window* window = (platform::window*)state;
    current_width = window->width;
    current_height = window->height;

    IDXGIFactory7* factory;
    UINT flags = 0;

#ifdef _DEBUG
    flags = DXGI_CREATE_FACTORY_DEBUG;

    ID3D12Debug6* debug_interface;
    HRCheck(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
    debug_interface->EnableDebugLayer();

    debug_interface->Release();
#endif

    HRCheck(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)));

    if(!d3d12_device::create(factory)) {
        FBERROR("An error ocurred during D3D12 device creation.");
        factory->Release();
        return false;
    }

    frame_index = 0;
    memory::fbzero(graphics_fence_values, sizeof(graphics_fence_values));
    memory::fbzero(compute_fence_values, sizeof(compute_fence_values));
    memory::fbzero(transfer_fence_values, sizeof(transfer_fence_values));

    graphics_queue.create(D3D12_COMMAND_LIST_TYPE_DIRECT);
    compute_queue.create(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    transfer_queue.create(D3D12_COMMAND_LIST_TYPE_COPY);

    dxgi_swapchain::create(factory, graphics_queue.get_queue(), window);

    factory->Release();

    FBINFO("D3D12 renderer backend initialized.");

    return true;
}

void d3d12_terminate() {
    transfer_queue.destroy();
    compute_queue.destroy();
    graphics_queue.destroy();

    dxgi_swapchain::destroy();
    d3d12_device::destroy();
}

void d3d12_begin_frame() {
}

void d3d12_end_frame() {
}

void d3d12_resize(u16 width, u16 height) {
    if (width != current_width || height != current_height) {
    }
}

void d3d12_update(f64 timestep) {
}

b8 d3d12_present() {
    return true;
}