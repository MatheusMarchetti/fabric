#pragma once

#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    class d3d12_resource {
       public:
        d3d12_resource() = default;
        d3d12_resource(const D3D12_RESOURCE_DESC1& desc, D3D12_CLEAR_VALUE* optimizedClear, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE accessType);
        d3d12_resource(ID3D12Resource2* resource, D3D12_RESOURCE_STATES currentState) : resource(resource), current_state(currentState) {}

        void release();

        ID3D12Resource2* get_resource() const { return resource; }
        D3D12_RESOURCE_STATES& state() { return current_state; }

       private:
        ID3D12Resource2* resource = nullptr;
        D3D12_RESOURCE_STATES current_state;
    };
}  // namespace fabric