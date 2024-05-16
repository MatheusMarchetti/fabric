#pragma once

#include "renderer/d3d12/d3d12_common.hpp"
#include "renderer/d3d12/d3d12_command_list.hpp"
#include "containers/darray.hpp"

namespace fabric {
    class d3d12_command_queue {
       public:
        void create(D3D12_COMMAND_LIST_TYPE type);
        void destroy();

        ID3D12CommandQueue* get_queue() { return command_queue; }
        ID3D12Fence1* get_fence() { return fence; }

        u64 signal();
        b8 is_fence_signaled(u64 fenceValue);

        void wait_for_queue(d3d12_command_queue& queue, u64 fenceValue);
        void wait_for_value(u64 fenceValue);
        void wait_for_idle();

        u64 submit(ID3D12CommandList* commandList);
        void submit(d3d12_command_list commandLists[], u32 count);

        const d3d12_command_list get_command_list();

        ID3D12CommandAllocator* recycle_allocator(ID3D12CommandAllocator* submittedAllocator, u64 submissionValue);

       private:
        struct allocator_submission {
            ID3D12CommandAllocator* allocator;
            u64 submission_value;
        };

        ID3D12CommandAllocator* create_command_allocator();

       private:
        D3D12_COMMAND_LIST_TYPE queue_type;
        u64 next_fence_value = 1;
        u64 last_completed_fence_value = 0;
        ID3D12CommandQueue* command_queue;
        ID3D12Fence1* fence;
        ftl::darray<ID3D12GraphicsCommandList7*> command_list_pool;
        ftl::darray<ID3D12GraphicsCommandList7*> available_lists;
        ftl::darray<ID3D12CommandAllocator*> allocator_pool;
        ftl::darray<ID3D12CommandAllocator*> available_allocators;
        ftl::darray<allocator_submission> submitted_allocators;
    };
}  // namespace fabric