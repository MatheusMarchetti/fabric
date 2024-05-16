#pragma once

#include "renderer/d3d12/d3d12_common.hpp"

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

        void set_render_targets(D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[], u32 count, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencil);
        void clear_render_target(D3D12_CPU_DESCRIPTOR_HANDLE renderTarget, f32 color[4]);
        void clear_depth_stencil(D3D12_CPU_DESCRIPTOR_HANDLE depthStencil, f32 depth, u8 stencil);
        void submit();

        void copy_resource(class d3d12_resource& source, class d3d12_resource& dest) const;

        void transition_barrier(class d3d12_resource* resource, const D3D12_RESOURCE_STATES afterState) const;
        void transition_barriers(class d3d12_resource* resources[], const D3D12_RESOURCE_STATES afterStates[], u32 count) const;

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