#include "renderer/d3d12/d3d12_context.hpp"
#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "core/logger.hpp"
#include "containers/darray.hpp"

using namespace fabric;

namespace {
    IDXGIAdapter4* physical_adapter;
    ID3D12Device10* logical_device;

    d3d12_descriptor_allocator descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    ftl::darray<ID3D12Resource2*> renderer_resources;
}  // namespace

void CALLBACK d3d12_report_validation(D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY, D3D12_MESSAGE_ID, LPCSTR, void*);
static DWORD callback_cookie;

b8 d3d12_context::create(IDXGIFactory7* dxgiFactory) {
    u8 i = 0;
    u8 best_adapter = 0;
    SIZE_T total_vram = 0;
    D3D_FEATURE_LEVEL max_supported_feature_level;
    IDXGIAdapter1* adapter;
    ID3D12Device* device;

    while (dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == 0 &&
            desc.DedicatedVideoMemory > total_vram) {
            for (u16 feature_level = D3D_FEATURE_LEVEL_12_2; feature_level >= D3D_FEATURE_LEVEL_12_0; feature_level -= 0x0100) {
                if (SUCCEEDED(D3D12CreateDevice(adapter, (D3D_FEATURE_LEVEL)feature_level, IID_PPV_ARGS(&device)))) {
                    u8 requirements = 0;

                    D3D12_FEATURE_DATA_D3D12_OPTIONS binding_tier;
                    binding_tier.ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_3;
                    requirements = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &binding_tier, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS))) ? (1 << 0) : 0;

                    D3D12_FEATURE_DATA_SHADER_MODEL shader_model;
                    shader_model.HighestShaderModel = D3D_SHADER_MODEL_6_6;
                    requirements = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(D3D12_FEATURE_SHADER_MODEL))) ? (1 << 1) : 0;

                    if (requirements != 0) {
                        max_supported_feature_level = (D3D_FEATURE_LEVEL)feature_level;
                        best_adapter = i;
                        total_vram = desc.DedicatedVideoMemory;
                    }

                    device->Release();
                    break;
                }
            }
        }

        adapter->Release();
        i++;
    }

    if (total_vram == 0) {
        FBERROR("A physical adapter with DirectX12 support (FEATURE_LEVEL_12_0 or above) is required");
        return false;
    }

    HRCheck(dxgiFactory->EnumAdapters1(best_adapter, &adapter));
    HRCheck(adapter->QueryInterface(&physical_adapter));
    adapter->Release();

    // TODO: Get adapter information

    HRESULT hr = D3D12CreateDevice(physical_adapter, max_supported_feature_level, IID_PPV_ARGS(&logical_device));

    if (FAILED(hr)) {
        FBERROR("Logical device creation failed with error %s", hr);
        return false;
    }

    NAME_OBJECTS(logical_device);

#ifdef _DEBUG
    ID3D12InfoQueue1* info_queue;
    if (SUCCEEDED(logical_device->QueryInterface(&info_queue))) {
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
        return false;
    }
#endif

    for (u8 heap_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; heap_type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; heap_type++) {
        descriptor_allocators[heap_type].create((D3D12_DESCRIPTOR_HEAP_TYPE)heap_type);
    }

    renderer_resources.reserve(64);

    return true;
}

void d3d12_context::destroy() {
    for (u64 i = 0; i < renderer_resources.length(); i++) {
        // NOTE: At this point, GPU must have finished any pending work, so this shouldn't ever fail.
        renderer_resources[i]->Release();
    }

    for (u8 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        descriptor_allocators[i].destroy();
    }

    logical_device->Release();
    physical_adapter->Release();
}

ID3D12Device10* d3d12_context::get_logical_device() {
    return logical_device;
}

handle<texture> d3d12_context::create_render_target(ID3D12Resource2* resource, D3D12_RENDER_TARGET_VIEW_DESC desc) {
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    texture.index = renderer_resources.length();
    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::render_target;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateRenderTargetView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_depth_stencil(ID3D12Resource2* resource, D3D12_DEPTH_STENCIL_VIEW_DESC desc) {
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();

    texture.index = renderer_resources.length();
    texture.depth_stencil = allocation.offset;
    texture.flags |= texture::flags::depth_stencil;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateDepthStencilView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_readonly_texture(ID3D12Resource2* resource, D3D12_SHADER_RESOURCE_VIEW_DESC desc) {
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.index = renderer_resources.length();
    texture.shader_resource = allocation.offset;
    texture.flags |= texture::flags::readonly;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateShaderResourceView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_writable_texture(ID3D12Resource2* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC desc) {
    handle<texture> texture;

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.index = renderer_resources.length();
    texture.shader_resource = allocation.offset;
    texture.flags |= texture::flags::writable;

    renderer_resources.push(resource);

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateUnorderedAccessView(resource, nullptr, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_render_target(handle<texture> texture, D3D12_RENDER_TARGET_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];

    if (texture.flags & texture::flags::render_target) {
        FBWARN("Trying to create a duplicate render target view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::render_target;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateRenderTargetView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_depth_stencil(handle<texture> texture, D3D12_DEPTH_STENCIL_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];

    if (texture.flags & texture::flags::depth_stencil) {
        FBWARN("Trying to create a duplicate depth stencil view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::depth_stencil;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateDepthStencilView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_readonly_texture(handle<texture> texture, D3D12_SHADER_RESOURCE_VIEW_DESC desc) {
    auto resource = renderer_resources[texture.index];

    if (texture.flags & texture::flags::readonly) {
        FBWARN("Trying to create a duplicate shader resource view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::readonly;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateShaderResourceView(resource, &desc, cpu_handle);

    return texture;
}

handle<texture> d3d12_context::create_writable_texture(handle<texture> texture, D3D12_UNORDERED_ACCESS_VIEW_DESC desc) {
        auto resource = renderer_resources[texture.index];

    if (texture.flags & texture::flags::writable) {
        FBWARN("Trying to create a duplicate unordered access view. Nothing is done.");

        return texture;
    }

    descriptor_allocation allocation = descriptor_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].allocate();

    texture.render_target = allocation.offset;
    texture.flags |= texture::flags::writable;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    cpu_handle.ptr = allocation.descriptor_handle;

    logical_device->CreateUnorderedAccessView(resource, nullptr, &desc, cpu_handle);

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