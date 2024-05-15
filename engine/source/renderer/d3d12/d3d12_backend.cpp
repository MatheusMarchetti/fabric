#include "renderer/d3d12/d3d12_backend.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_command_list.hpp"
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

    d3d12_command_list graphics_list;

    d3d12_descriptor_allocator descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    ftl::darray<backend::resource> renderer_resources;
}  // namespace

void CALLBACK d3d12_report_validation(D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY, D3D12_MESSAGE_ID, LPCSTR, void*);
static DWORD callback_cookie;

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

#ifdef _DEBUG
    auto device = d3d12_device::get_logical_device();
    ID3D12InfoQueue1* info_queue;
    if (SUCCEEDED(device->QueryInterface(&info_queue))) {
        D3D12_MESSAGE_ID deny[] =
            {
                D3D12_MESSAGE_ID_CREATE_COMMANDQUEUE,
                D3D12_MESSAGE_ID_CREATE_COMMANDALLOCATOR,
                D3D12_MESSAGE_ID_CREATE_PIPELINESTATE,
                D3D12_MESSAGE_ID_CREATE_COMMANDLIST12,
                D3D12_MESSAGE_ID_CREATE_RESOURCE,
                D3D12_MESSAGE_ID_CREATE_DESCRIPTORHEAP,
                D3D12_MESSAGE_ID_CREATE_ROOTSIGNATURE,
                D3D12_MESSAGE_ID_CREATE_LIBRARY,
                D3D12_MESSAGE_ID_CREATE_HEAP,
                D3D12_MESSAGE_ID_CREATE_MONITOREDFENCE,
                D3D12_MESSAGE_ID_CREATE_QUERYHEAP,
                D3D12_MESSAGE_ID_CREATE_COMMANDSIGNATURE,
                D3D12_MESSAGE_ID_CREATE_LIFETIMETRACKER,
                D3D12_MESSAGE_ID_CREATE_SHADERCACHESESSION,
                D3D12_MESSAGE_ID_DESTROY_COMMANDQUEUE,
                D3D12_MESSAGE_ID_DESTROY_COMMANDALLOCATOR,
                D3D12_MESSAGE_ID_DESTROY_PIPELINESTATE,
                D3D12_MESSAGE_ID_DESTROY_COMMANDLIST12,
                D3D12_MESSAGE_ID_DESTROY_RESOURCE,
                D3D12_MESSAGE_ID_DESTROY_DESCRIPTORHEAP,
                D3D12_MESSAGE_ID_DESTROY_ROOTSIGNATURE,
                D3D12_MESSAGE_ID_DESTROY_LIBRARY,
                D3D12_MESSAGE_ID_DESTROY_HEAP,
                D3D12_MESSAGE_ID_DESTROY_MONITOREDFENCE,
                D3D12_MESSAGE_ID_DESTROY_QUERYHEAP,
                D3D12_MESSAGE_ID_DESTROY_COMMANDSIGNATURE,
                D3D12_MESSAGE_ID_DESTROY_LIFETIMETRACKER,
                D3D12_MESSAGE_ID_DESTROY_SHADERCACHESESSION};

        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(deny);
        filter.DenyList.pIDList = deny;

        HRCheck(info_queue->PushStorageFilter(&filter));
        HRCheck(info_queue->RegisterMessageCallback(d3d12_report_validation, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &callback_cookie));
        info_queue->Release();
    } else {
        FBERROR("An error ocurred while setting up D3D12 validation.");
        factory->Release();
        return false;
    }
#endif

    for (u8 heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; heap_type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; heap_type++) {
        descriptor_allocators[heap_type].create((D3D12_DESCRIPTOR_HEAP_TYPE)heap_type);
    }

    frame_index = 1;
    memory::fbzero(graphics_fence_values, sizeof(graphics_fence_values));
    memory::fbzero(compute_fence_values, sizeof(compute_fence_values));
    memory::fbzero(transfer_fence_values, sizeof(transfer_fence_values));

    graphics_queue.create(D3D12_COMMAND_LIST_TYPE_DIRECT);
    compute_queue.create(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    transfer_queue.create(D3D12_COMMAND_LIST_TYPE_COPY);

    graphics_list = graphics_queue.get_command_list();

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
        renderer_resources[i].resource->Release();
    }

    for (u8 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        descriptor_allocators[i].destroy();
    }

    dxgi_swapchain::destroy();
    d3d12_device::destroy();
}

void d3d12_begin_frame() {
    frame_index = (frame_index + 1) % frames_in_flight;

    graphics_queue.wait_for_value(graphics_fence_values[frame_index]);
    // compute_queue.wait_for_value(compute_fence_values[frame_index]);
    // transfer_queue.wait_for_value(transfer_fence_values[frame_index]);

    dxgi_swapchain::begin_frame(graphics_list);

    handle<texture> swapchain_buffer = dxgi_swapchain::get_current_image();
    handle<texture> render_targets[] = {swapchain_buffer};
    float clear_color[4] = {0.2f, 0.2f, 0.2f, 1.0f};

    graphics_list.clear_render_target(dxgi_swapchain::get_current_image(), clear_color);
    graphics_list.set_render_targets(render_targets, _countof(render_targets));
}

void d3d12_end_frame() {
    dxgi_swapchain::end_frame(graphics_list);
    graphics_list.submit();
}

void d3d12_resize(u16 width, u16 height) {
    graphics_queue.wait_for_idle();

    if (width != current_width || height != current_height) {
        current_width = width;
        current_height = height;
        dxgi_swapchain::resize(width, height);
    }
}

void d3d12_update(f64 timestep) {
}

b8 d3d12_present() {
    b8 result = dxgi_swapchain::present(false);
    graphics_fence_values[frame_index] = graphics_queue.signal();

    return result;
}

d3d12_descriptor_allocator& backend::get_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    return descriptor_allocators[type];
}

backend::resource& backend::get_resource(u32 index) {
    return renderer_resources[index];
}

u32 backend::register_resource(ID3D12Resource2* resource, D3D12_RESOURCE_STATES state) {
    renderer_resources.push({resource, state});
    return renderer_resources.length() - 1;
}

handle<texture> backend::create_render_target(ID3D12Resource2* resource, D3D12_RENDER_TARGET_VIEW_DESC desc) {
    auto device = d3d12_device::get_logical_device();
    handle<texture> tex;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    tex.render_target = allocation.offset;
    tex.flags |= texture::flags::render_target;

    device->CreateRenderTargetView(resource, &desc, allocation.cpu_handle);

    return tex;
}

handle<texture> backend::create_render_target(handle<texture> texture, D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource2* resource) {
    if(!resource) {
        resource = renderer_resources[texture.index].resource;
    }
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::render_target) {
        FBINFO("Recreating the render target view");

        auto cpu_handle = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].get_cpu_handle(texture.render_target);
        device->CreateRenderTargetView(resource, &desc, cpu_handle);

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::render_target;

    device->CreateRenderTargetView(resource, &desc, allocation.cpu_handle);

    return texture;
}

handle<texture> backend::create_depth_stencil(handle<texture> texture, D3D12_DEPTH_STENCIL_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index].resource;
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::depth_stencil) {
        FBWARN("Trying to create a duplicate depth stencil view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::depth_stencil;

    device->CreateDepthStencilView(resource, &desc, allocation.cpu_handle);

    return texture;
}

handle<texture> backend::create_readonly_texture(handle<texture> texture, D3D12_SHADER_RESOURCE_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index].resource;
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::readonly) {
        FBWARN("Trying to create a duplicate shader resource view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::readonly;

    device->CreateShaderResourceView(resource, &desc, allocation.cpu_handle);

    return texture;
}

handle<texture> backend::create_writable_texture(handle<texture> texture, D3D12_UNORDERED_ACCESS_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index].resource;
    auto device = d3d12_device::get_logical_device();

    if (texture.flags & texture::flags::writable) {
        FBWARN("Trying to create a duplicate unordered access view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::writable;

    device->CreateUnorderedAccessView(resource, nullptr, &desc, allocation.cpu_handle);

    return texture;
}

void CALLBACK d3d12_report_validation(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID messageID, LPCSTR message, void* context) {
    switch (severity) {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            FBFATAL(message);
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            FBERROR(message);
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            FBWARN(message);
            break;
        case D3D12_MESSAGE_SEVERITY_INFO:
            FBINFO(message);
            break;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            FBDEBUG(message);
            break;
    }
}