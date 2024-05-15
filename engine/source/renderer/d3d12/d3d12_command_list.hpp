#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "renderer/resources.hpp"

namespace fabric {
    class d3d12_command_list {
        friend class d3d12_command_queue;

       public:
        d3d12_command_list() = default;
        d3d12_command_list(ID3D12GraphicsCommandList7* commandList, ID3D12CommandAllocator* commandAllocator, d3d12_command_queue* commandQueue);
        d3d12_command_list(const d3d12_command_list&) = delete;
        d3d12_command_list& operator=(const d3d12_command_list& rhs);
        d3d12_command_list(d3d12_command_list&&) = delete;
        d3d12_command_list& operator=(d3d12_command_list&&) = delete;
        ~d3d12_command_list() = default;

        void set_render_targets(handle<texture> renderTargets[], u32 count);
        void clear_render_target(const handle<texture>& renderTarget, float color[4]);
        void submit();

        void transition_barrier(ID3D12Resource2* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const;
        void transition_barriers(ID3D12Resource2* resources[], D3D12_RESOURCE_STATES beforeStates[], D3D12_RESOURCE_STATES afterStates[], u32 count) const;

       private:
        ID3D12GraphicsCommandList7* get_command_list() { return command_list; }
        ID3D12CommandAllocator* get_command_allocator() { return command_allocator; }
        void reset();

       private:
        ID3D12GraphicsCommandList7* command_list;
        ID3D12CommandAllocator* command_allocator;
        class d3d12_command_queue* queue;
    };
}  // namespace fabric