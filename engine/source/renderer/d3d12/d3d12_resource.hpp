#pragma once

#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    class d3d12_resource {
       public:
        d3d12_resource() = default;
        d3d12_resource(const D3D12_RESOURCE_DESC1& desc, D3D12_CLEAR_VALUE* optimizedClear, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE accessType);
        d3d12_resource(ID3D12Resource2* resource, D3D12_RESOURCE_STATES currentState) : resource(resource), current_state(currentState) {}

        void release();
        void resize(u16 width, u16 height);

        ID3D12Resource2* get_resource() const { return resource; }
        D3D12_RESOURCE_STATES& state() { return current_state; }
        D3D12_CPU_DESCRIPTOR_HANDLE& handle(D3D12_DESCRIPTOR_HEAP_TYPE type) { return descriptor_handles[type]; }
        u32& index() { return resource_index; }

       private:
        u32 resource_index = -1;
        ID3D12Resource2* resource = nullptr;
        D3D12_RESOURCE_STATES current_state;
        D3D12_RESOURCE_STATES initial_state;
        D3D12_CLEAR_VALUE optimized_clear;
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handles[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {D3D12_CPU_DESCRIPTOR_HANDLE{0}};
    };
}  // namespace fabric