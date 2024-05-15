#include "renderer/d3d12/d3d12_command_list.hpp"
#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "renderer/d3d12/d3d12_backend.hpp"

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

void d3d12_command_list::set_render_targets(handle<texture> renderTargets[], u32 count) {
    ftl::darray<D3D12_CPU_DESCRIPTOR_HANDLE> render_targets(count);

    for (u32 i = 0; i < count; i++) {
        auto& descriptor_allocator = backend::get_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        render_targets.push(descriptor_allocator.get_cpu_handle(renderTargets[i].render_target));
    }

    command_list->OMSetRenderTargets(count, render_targets.data(), false, NULL);
}

void d3d12_command_list::clear_render_target(const handle<texture>& renderTarget, float color[4]) {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor;
    auto& descriptor_allocator = backend::get_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    cpu_descriptor = descriptor_allocator.get_cpu_handle(renderTarget.render_target);

    command_list->ClearRenderTargetView(cpu_descriptor, color, 0, nullptr);
}

void d3d12_command_list::submit() {
    HRCheck(command_list->Close());

    u64 submission_value = queue->submit(command_list);
    command_allocator = queue->recycle_allocator(command_allocator, submission_value);
    command_list->Reset(command_allocator, nullptr);
}

void d3d12_command_list::transition_barrier(ID3D12Resource2* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const {
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateAfter = afterState;
    barrier.Transition.StateBefore = beforeState;
    barrier.Transition.pResource = resource;

    command_list->ResourceBarrier(1, &barrier);
}

void d3d12_command_list::transition_barriers(ID3D12Resource2* resources[], D3D12_RESOURCE_STATES beforeStates[], D3D12_RESOURCE_STATES afterStates[], u32 count) const {
    ftl::darray<D3D12_RESOURCE_BARRIER> barriers(count);
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    for (u32 i = 0; i < count; i++) {
        barrier.Transition.pResource = resources[i];
        barrier.Transition.StateAfter = afterStates[i];
        barrier.Transition.StateBefore = beforeStates[i];

        barriers.push(barrier);
    }

    command_list->ResourceBarrier(barriers.length(), barriers.data());
}

void d3d12_command_list::reset() {
    command_list = nullptr;
    command_allocator = nullptr;
}