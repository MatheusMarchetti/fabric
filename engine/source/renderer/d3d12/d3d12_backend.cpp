#include "renderer/d3d12/d3d12_backend.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_command_list.hpp"
#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "renderer/d3d12/d3d12_device.hpp"
#include "renderer/d3d12/d3d12_interface.hpp"
#include "renderer/d3d12/d3d12_resource.hpp"
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
    d3d12_resource main_color;
    descriptor_allocation main_color_rtv;
    u32 main_color_index;
    d3d12_resource main_depth;
    descriptor_allocation main_depth_dsv;
    u32 main_depth_index;

    d3d12_descriptor_allocator descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    ftl::darray<d3d12_resource> renderer_resources;
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

    dxgi_swapchain::create(factory, graphics_queue.get_queue(), window);

    factory->Release();

    graphics_list = graphics_queue.get_command_list();

    main_color_index = renderer_resources.length();
    main_color = backend::create_texture2D(current_width, current_height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D};

    main_color_rtv = backend::create_render_target_view(main_color.get_resource(), rtv_desc);

    main_depth_index = renderer_resources.length();
    main_depth = backend::create_depth_stencil(current_width, current_height, false);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
        .Format = DXGI_FORMAT_D32_FLOAT,
        .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D};

    main_depth_dsv = backend::create_depth_stencil_view(main_depth.get_resource(), dsv_desc);

    FBINFO("D3D12 renderer backend initialized.");

    return true;
}

void d3d12_terminate() {
    transfer_queue.destroy();
    compute_queue.destroy();
    graphics_queue.destroy();

    main_color = d3d12_resource();
    main_depth = d3d12_resource();

    for (u32 i = 0; i < renderer_resources.length(); i++) {
        renderer_resources[i].release();
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
    compute_queue.wait_for_value(compute_fence_values[frame_index]);
    transfer_queue.wait_for_value(transfer_fence_values[frame_index]);

    graphics_list.transition_barrier(&main_color, D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_CPU_DESCRIPTOR_HANDLE render_targets[] = {main_color_rtv.cpu_handle};
    f32 clear_color[4] = {0.2f, 0.2f, 0.2f, 1.0f};

    graphics_list.clear_render_target(main_color_rtv.cpu_handle, clear_color);
    graphics_list.clear_depth_stencil(main_depth_dsv.cpu_handle, 1.0f, 0);
    graphics_list.set_render_targets(render_targets, _countof(render_targets), &main_depth_dsv.cpu_handle);
}

void d3d12_end_frame() {
    dxgi_swapchain::end_frame(graphics_list, main_color);
    graphics_list.submit();
}

void d3d12_resize(u16 width, u16 height) {
    graphics_queue.wait_for_idle();

    if (width != current_width || height != current_height) {
        current_width = width;
        current_height = height;

        renderer_resources[main_color_index].release();
        renderer_resources[main_depth_index].release();
        renderer_resources.pop(main_color_index);
        renderer_resources.pop(main_depth_index - 1);

        main_color_index = renderer_resources.length();
        main_color = backend::create_texture2D(current_width, current_height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D};

        main_color_rtv = backend::create_render_target_view(main_color.get_resource(), rtv_desc, main_color_rtv.offset);

        main_depth_index = renderer_resources.length();
        main_depth = backend::create_depth_stencil(current_width, current_height, false);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D};

        main_depth_dsv = backend::create_depth_stencil_view(main_depth.get_resource(), dsv_desc, main_depth_dsv.offset);

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

const d3d12_resource& backend::create_texture2D(u16 width, u16 height, DXGI_FORMAT format, u8 mipLevels) {
    D3D12_RESOURCE_DESC1 desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = mipLevels;
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.SampleDesc = {1, 0};
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    desc.SamplerFeedbackMipRegion = {0, 0, 0};

    D3D12_CLEAR_VALUE clear = {
        .Format = format,
        .Color = {0.2f, 0.2f, 0.2f, 1.0f}};

    return renderer_resources.push(d3d12_resource(desc, &clear, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT));
}

const d3d12_resource& backend::create_depth_stencil(u16 width, u16 height, b8 useStencil) {
    D3D12_RESOURCE_DESC1 desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Format = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_R32_TYPELESS;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.SampleDesc = {1, 0};
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | ((useStencil) ? D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE : D3D12_RESOURCE_FLAG_NONE);
    desc.SamplerFeedbackMipRegion = {0, 0, 0};

    D3D12_CLEAR_VALUE clear = {
        .Format = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT,
        .DepthStencil = {
            .Depth = 1.0f,
            .Stencil = 0}};

    return renderer_resources.push(d3d12_resource(desc, &clear, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_HEAP_TYPE_DEFAULT));
}

descriptor_allocation backend::create_render_target_view(ID3D12Resource2* resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, u32 offset) {
    auto device = d3d12_device::get_logical_device();
    descriptor_allocation allocation;

    if (offset != -1) {
        allocation.offset = offset;
        allocation.cpu_handle = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].get_cpu_handle(offset);
    } else {
        allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();
    }

    device->CreateRenderTargetView(resource, &desc, allocation.cpu_handle);

    return allocation;
}

descriptor_allocation backend::create_depth_stencil_view(ID3D12Resource2* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, u32 offset) {
    auto device = d3d12_device::get_logical_device();
    descriptor_allocation allocation;

    if (offset != -1) {
        allocation.offset = offset;
        allocation.cpu_handle = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].get_cpu_handle(offset);
    } else {
        allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();
    }

    device->CreateDepthStencilView(resource, &desc, allocation.cpu_handle);

    return allocation;
}

descriptor_allocation backend::create_readonly_texture_view(ID3D12Resource2* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, u32 offset) {
    auto device = d3d12_device::get_logical_device();
    descriptor_allocation allocation;

    if (offset != -1) {
        allocation.offset = offset;
        allocation.cpu_handle = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get_cpu_handle(offset);
    } else {
        allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();
    }

    device->CreateShaderResourceView(resource, &desc, allocation.cpu_handle);

    return allocation;
}

descriptor_allocation backend::create_writable_texture_view(ID3D12Resource2* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, u32 offset) {
    auto device = d3d12_device::get_logical_device();
    descriptor_allocation allocation;

    if (offset != -1) {
        allocation.offset = offset;
        allocation.cpu_handle = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].get_cpu_handle(offset);
    } else {
        allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();
    }

    device->CreateUnorderedAccessView(resource, nullptr, &desc, allocation.cpu_handle);

    return allocation;
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