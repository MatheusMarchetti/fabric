#include "renderer/d3d12/d3d12_command_queue.hpp"
#include "renderer/d3d12/d3d12_device.hpp"
#include "core/memory.hpp"
#include "ftl/math.hpp"

using namespace fabric;

void d3d12_command_queue::create(D3D12_COMMAND_LIST_TYPE type) {
    queue_type = type;

    D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0};

    auto device = d3d12_device::get_logical_device();

    HRCheck(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));
    HRCheck(device->CreateFence(last_completed_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    fence->Signal(last_completed_fence_value);

    switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            NAME_OBJECTS_TYPE(command_queue, graphics_queue);
            NAME_OBJECTS_TYPE(fence, graphics_fence);
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            NAME_OBJECTS_TYPE(command_queue, compute_queue);
            NAME_OBJECTS_TYPE(fence, compute_fence);
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            NAME_OBJECTS_TYPE(command_queue, transfer_queue);
            NAME_OBJECTS_TYPE(fence, transfer_fence);
            break;
        default:
            break;
    }
}

void d3d12_command_queue::destroy() {
    wait_for_idle();

    for (u32 i = 0; i < command_list_pool.length(); i++) {
        command_list_pool[i]->Release();
    }

    for (u32 i = 0; i < allocator_pool.length(); i++) {
        allocator_pool[i]->Release();
    }

    command_queue->Release();
    fence->Release();
}

u64 d3d12_command_queue::signal() {
    HRCheck(command_queue->Signal(fence, next_fence_value));

    return next_fence_value++;
}

b8 d3d12_command_queue::is_fence_signaled(u64 fenceValue) {
    if (fenceValue > last_completed_fence_value) {
        last_completed_fence_value = ftl::max(last_completed_fence_value, fence->GetCompletedValue());
    }

    return fenceValue <= last_completed_fence_value;
}

void d3d12_command_queue::wait_for_queue(d3d12_command_queue& queue, u64 fenceValue) {
    auto fence_to_wait = queue.get_fence();
    HRCheck(command_queue->Wait(fence_to_wait, fenceValue));
}

void d3d12_command_queue::wait_for_value(u64 fenceValue) {
    if (!is_fence_signaled(fenceValue)) {
        HANDLE event = CreateEventExA(NULL, NULL, NULL, NULL);
        HRCheck(fence->SetEventOnCompletion(fenceValue, event));
        WaitForSingleObjectEx(event, INFINITE, false);
        CloseHandle(event);

        last_completed_fence_value = fenceValue;
    }

    while (!submitted_allocators.empty() && submitted_allocators[0].submission_value <= fenceValue) {
        ID3D12CommandAllocator* allocator = submitted_allocators[0].allocator;
        HRCheck(allocator->Reset());
        available_allocators.push(allocator);
        submitted_allocators.pop(0);
    }
}

void d3d12_command_queue::wait_for_idle() {
    wait_for_value(next_fence_value - 1);
}

u64 d3d12_command_queue::submit(d3d12_command_list* commandLists, u32 count) {
    ftl::darray<ID3D12CommandList*> command_lists(count);

    for (u32 i = 0; i < count; i++) {
        auto commandList = commandLists[i].get_command_list();
        HRCheck(commandList->Close());
        command_lists.push(commandList);
    }

    command_queue->ExecuteCommandLists(command_lists.length(), command_lists.data());
    u64 submission_value = signal();

    for (u32 i = 0; i < count; i++) {
        auto commandList = commandLists[i].get_command_list();
        auto commandAllocator = commandLists[i].get_command_allocator();

        allocator_submission submission{
            .allocator = commandAllocator,
            .submission_value = submission_value};

        submitted_allocators.push(submission);
        available_lists.push(commandList);

        commandLists[i].reset();
    }

    return submission_value;
}

const d3d12_command_list d3d12_command_queue::get_command_list() {
    ID3D12GraphicsCommandList7* command_list;
    ID3D12CommandAllocator* allocator;

    if (!available_allocators.empty()) {
        allocator = available_allocators[available_allocators.length() - 1];
        available_allocators.pop();
    } else {
        allocator = create_command_allocator();
    }

    if (!available_lists.empty()) {
        command_list = available_lists[available_lists.length() - 1];
        available_lists.pop();

        command_list->Reset(allocator, nullptr);
    } else {
        auto device = d3d12_device::get_logical_device();
        HRCheck(device->CreateCommandList(0, queue_type, allocator, nullptr, IID_PPV_ARGS(&command_list)));
        command_list_pool.push(command_list);
    }

    return d3d12_command_list(command_list, allocator, this);
}

ID3D12CommandAllocator* d3d12_command_queue::create_command_allocator() {
    ID3D12CommandAllocator* allocator;

    auto device = d3d12_device::get_logical_device();
    HRCheck(device->CreateCommandAllocator(queue_type, IID_PPV_ARGS(&allocator)));
    allocator_pool.push(allocator);

    return allocator;
}