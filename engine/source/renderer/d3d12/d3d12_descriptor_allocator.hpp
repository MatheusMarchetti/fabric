#pragma once

#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    class d3d12_descriptor_allocator {
       public:
        void create(D3D12_DESCRIPTOR_HEAP_TYPE type);
        void destroy();

        ID3D12DescriptorHeap* get_heap() { return descriptor_heap; }

        descriptor_allocation allocate(u64 count = 1);

        D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(u32 descriptorOffset);

       private:
        ID3D12DescriptorHeap* descriptor_heap;
        D3D12_DESCRIPTOR_HEAP_TYPE heap_type;
        u64 descriptor_size;
        u64 descriptor_offset;
    };
}  // namespace fabric