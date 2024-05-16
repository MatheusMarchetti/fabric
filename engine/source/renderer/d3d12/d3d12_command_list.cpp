#include "renderer/d3d12/d3d12_command_list.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_resource.hpp"

using namespace fabric;

d3d12_command_list::d3d12_command_list(ID3D12GraphicsCommandList7* commandList, ID3D12CommandAllocator* commandAllocator, d3d12_command_queue* commandQueue) {
    command_list = commandList;
    command_allocator = commandAllocator;
    queue = commandQueue;
}

d3d12_command_list& d3d12_command_list::operator=(const d3d12_command_list& rhs) {
    command_list = rhs.command_list;
    command_allocator = rhs.command_allocator;
    queue = rhs.queue;

    return *this;
}

void d3d12_command_list::set_render_targets(D3D12_CPU_DESCRIPTOR_HANDLE renderTargets[], u32 count, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencil) {
    command_list->OMSetRenderTargets(count, renderTargets, false, depthStencil);
}

void d3d12_command_list::clear_render_target(D3D12_CPU_DESCRIPTOR_HANDLE renderTarget, float color[4]) {
    command_list->ClearRenderTargetView(renderTarget, color, 0, nullptr);
}

void d3d12_command_list::clear_depth_stencil(D3D12_CPU_DESCRIPTOR_HANDLE depthStencil, f32 depth, u8 stencil) {
    command_list->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
}

void d3d12_command_list::submit() {
    HRCheck(command_list->Close());

    u64 submission_value = queue->submit(command_list);
    command_allocator = queue->recycle_allocator(command_allocator, submission_value);
    command_list->Reset(command_allocator, nullptr);
}

void d3d12_command_list::copy_resource(class d3d12_resource& source, class d3d12_resource& dest) const {
    command_list->CopyResource(dest.get_resource(), source.get_resource());
}

void d3d12_command_list::transition_barrier(d3d12_resource* resource, D3D12_RESOURCE_STATES afterState) const {
    auto& current_state = resource->state();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateAfter = afterState;
    barrier.Transition.StateBefore = current_state;
    barrier.Transition.pResource = resource->get_resource();

    command_list->ResourceBarrier(1, &barrier);
    current_state = afterState;
}

void d3d12_command_list::transition_barriers(d3d12_resource* resources[], const D3D12_RESOURCE_STATES afterStates[], u32 count) const {
    ftl::darray<D3D12_RESOURCE_BARRIER> barriers(count);
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    for (u32 i = 0; i < count; i++) {
        auto& current_state = resources[i]->state();
        barrier.Transition.pResource = resources[i]->get_resource();
        barrier.Transition.StateAfter = afterStates[i];
        barrier.Transition.StateBefore = current_state;

        barriers.push(barrier);
        current_state = afterStates[i];
    }

    command_list->ResourceBarrier(barriers.length(), barriers.data());
}

void d3d12_command_list::reset() {
    command_list = nullptr;
    command_allocator = nullptr;
}