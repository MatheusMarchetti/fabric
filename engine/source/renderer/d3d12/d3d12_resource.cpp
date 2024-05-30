#include "renderer/d3d12/d3d12_resource.hpp"
#include "renderer/d3d12/d3d12_device.hpp"
#include "core/asserts.hpp"
#include "core/memory.hpp"

using namespace fabric;

d3d12_resource::d3d12_resource(const D3D12_RESOURCE_DESC1& desc, D3D12_CLEAR_VALUE* optimizedClear, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE accessType)
    : current_state(initialState), initial_state(initialState), optimized_clear(*optimizedClear) {
    auto device = d3d12_device::get_logical_device();

    FBASSERT_MSG(accessType != D3D12_HEAP_TYPE_CUSTOM, "D3D12_HEAP_TYPE_CUSTOM not supported yet.");

    D3D12_HEAP_PROPERTIES heap_properties;
    heap_properties.Type = accessType;
    heap_properties.CreationNodeMask = 0;
    heap_properties.VisibleNodeMask = 0;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    HRCheck(device->CreateCommittedResource2(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, initialState, optimizedClear, nullptr, IID_PPV_ARGS(&resource)));
}

void d3d12_resource::release() {
    resource->Release();
}

void d3d12_resource::resize(u16 width, u16 height) {
    D3D12_RESOURCE_DESC1 desc = resource->GetDesc1();
    D3D12_HEAP_PROPERTIES heap_properties;
    D3D12_HEAP_FLAGS heap_flags;
    HRCheck(resource->GetHeapProperties(&heap_properties, &heap_flags));

    resource->Release();
    auto device = d3d12_device::get_logical_device();

    desc.Width = width;
    desc.Height = height;
    current_state = initial_state;

    HRCheck(device->CreateCommittedResource2(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, initial_state, &optimized_clear, nullptr, IID_PPV_ARGS(&resource)));
}