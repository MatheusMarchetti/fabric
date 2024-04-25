#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_context.hpp"
#include "core/memory.hpp"

using namespace fabric;

void d3d12_command_queue::create(D3D12_COMMAND_LIST_TYPE type) {
    D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0};

    auto device = d3d12_context::get_logical_device();

    HRCheck(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));
    HRCheck(device->CreateFence(last_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    
    switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            NAME_OBJECTS_TYPE(command_queue, graphics_queue);
            NAME_OBJECTS_TYPE(fence, graphics_fence);
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            NAME_OBJECTS_TYPE(fence, compute_fence);
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            NAME_OBJECTS_TYPE(fence, transfer_fence);
            break;
        default:
        break;
    }
}

void d3d12_command_queue::destroy() {
    // TODO: Wait on GPU work to finish before releasing resources

    command_queue->Release();
    fence->Release();
}