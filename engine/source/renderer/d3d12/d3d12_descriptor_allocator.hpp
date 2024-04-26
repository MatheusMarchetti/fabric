#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    struct descriptor_allocation {
        u64 offset;
        u64 descriptor_handle;
    };

    class d3d12_descriptor_allocator {
       public:
        void create(D3D12_DESCRIPTOR_HEAP_TYPE type);
        void destroy();

        ID3D12DescriptorHeap* get_heap() { return descriptor_heap; }

        descriptor_allocation allocate(u64 count = 1);

       private:
        ID3D12DescriptorHeap* descriptor_heap;
        u64 descriptor_offset;
    };
}  // namespace fabric