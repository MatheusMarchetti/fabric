#include "renderer/d3d12/d3d12_backend.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "renderer/d3d12/d3d12_device.hpp"
#include "renderer/d3d12/dxgi_swapchain.hpp"
#include "core/engine.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "containers/darray.hpp"
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

    d3d12_descriptor_allocator descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    ftl::darray<ID3D12Resource2*> renderer_resources;
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

    if (!d3d12_device::create(factory)) {
        FBERROR("An error ocurred during D3D12 device creation.");
        factory->Release();
        return false;
    }

    for (u8 heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; heap_type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; heap_type++) {
        descriptor_allocators[heap_type].create((D3D12_DESCRIPTOR_HEAP_TYPE)heap_type);
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

    for (u64 i = 0; i < renderer_resources.length(); i++) {
        renderer_resources[i]->Release();
    }

    for (u8 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        descriptor_allocators[i].destroy();
    }

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

handle<texture> backend::create_render_target(ID3D12Resource2* resource, D3D12_RENDER_TARGET_VIEW_DESC desc) {
    auto device = d3d12_device::get_logical_device();
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    texture.index = renderer_resources.length();
    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::render_target;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateRenderTargetView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_depth_stencil(ID3D12Resource2* resource, D3D12_DEPTH_STENCIL_VIEW_DESC desc) {
    auto device = d3d12_device::get_logical_device();
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();

    texture.index = renderer_resources.length();
    texture.depth_stencil = allocation.offset;
    texture.flags |= texture::flags::depth_stencil;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateDepthStencilView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_readonly_texture(ID3D12Resource2* resource, D3D12_SHADER_RESOURCE_VIEW_DESC desc) {
    auto device = d3d12_device::get_logical_device();
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.index = renderer_resources.length();
    texture.shader_resource = allocation.offset;
    texture.flags |= texture::flags::readonly;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateShaderResourceView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_writable_texture(ID3D12Resource2* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC desc) {
    auto device = d3d12_device::get_logical_device();
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.index = renderer_resources.length();
    texture.shader_resource = allocation.offset;
    texture.flags |= texture::flags::writable;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateUnorderedAccessView(resource, nullptr, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_render_target(handle<texture> texture, D3D12_RENDER_TARGET_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::render_target) {
        FBWARN("Trying to create a duplicate render target view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::render_target;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateRenderTargetView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_depth_stencil(handle<texture> texture, D3D12_DEPTH_STENCIL_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::depth_stencil) {
        FBWARN("Trying to create a duplicate depth stencil view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::depth_stencil;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateDepthStencilView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_readonly_texture(handle<texture> texture, D3D12_SHADER_RESOURCE_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::readonly) {
        FBWARN("Trying to create a duplicate shader resource view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::readonly;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateShaderResourceView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> backend::create_writable_texture(handle<texture> texture, D3D12_UNORDERED_ACCESS_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::writable) {
        FBWARN("Trying to create a duplicate unordered access view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::writable;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    device->CreateUnorderedAccessView(resource, nullptr, &desc, cpu_handle);

    return texture;
}