#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    class d3d12_command_queue {
       public:
        void create(D3D12_COMMAND_LIST_TYPE type);
        void destroy();

        ID3D12CommandQueue* get_queue() { return command_queue; }
        ID3D12Fence1* get_fence() { return fence; }

       private:
        // u64 next_fence_value = 1;
        u64 last_fence_value = 0;
        ID3D12CommandQueue* command_queue;
        ID3D12Fence1* fence;
    };
}  // namespace fabric