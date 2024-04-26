#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "renderer/d3d12/d3d12_device.hpp"

using namespace fabric;

void d3d12_descriptor_allocator::create(D3D12_DESCRIPTOR_HEAP_TYPE type) {
    u32 count = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? 1'000'000 : (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ? 2048
                                                                                                                            : 1024;
    D3D12_DESCRIPTOR_HEAP_FLAGS flags = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
        .Type = type,
        .NumDescriptors = count,
        .Flags = flags,
        .NodeMask = 0};

    auto device = d3d12_device::get_logical_device();
    HRCheck(device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap)));

    switch (type) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            NAME_OBJECTS_TYPE(descriptor_heap, cbv_srv_uav_heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            NAME_OBJECTS_TYPE(descriptor_heap, sampler_heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            NAME_OBJECTS_TYPE(descriptor_heap, rtv_heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            NAME_OBJECTS_TYPE(descriptor_heap, dsv_heap);
            break;
        default:
            break;
    }

    descriptor_offset = 0;
}

void d3d12_descriptor_allocator::destroy() {
    descriptor_heap->Release();
}

descriptor_allocation d3d12_descriptor_allocator::allocate(u64 count) {
    auto device = d3d12_device::get_logical_device();
    D3D12_DESCRIPTOR_HEAP_DESC desc = descriptor_heap->GetDesc();

    u64 descriptor_size = device->GetDescriptorHandleIncrementSize(desc.Type);

    descriptor_allocation allocation;
    allocation.offset = descriptor_offset;
    allocation.descriptor_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr + descriptor_size * descriptor_offset;

    descriptor_offset += count;

    return allocation;
}