#include "containers/darray.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

using namespace fabric;
using namespace ftl;

void internal::darray_create(internal::memory_layout& layout, u64 capacity, u64 stride) {
    u64 array_size = capacity * stride;

    layout.capacity = capacity;
    layout.block = memory::fballocate(array_size, memory::MEMORY_TAG_DARRAY);
}

void internal::darray_destroy(internal::memory_layout& layout, u64 stride) {
    u64 array_size = layout.capacity * stride;
    memory::fbfree(layout.block, array_size, memory::MEMORY_TAG_DARRAY);
    layout.capacity = 0;
    layout.length = 0;
}

void internal::darray_copy(internal::memory_layout& src, internal::memory_layout& dst, u64 stride) {
    internal::darray_create(dst, src.capacity, stride);

    memory::fbcopy(dst.block, src.block, src.length * stride);
    dst.length = src.length;
}

void internal::darray_resize(internal::memory_layout& layout, u64 newSize, u64 stride) {
    internal::memory_layout temp;
    internal::darray_create(temp, newSize, stride);

    temp.length = layout.length;

    memory::fbcopy(temp.block, layout.block, layout.length * stride);

    internal::darray_destroy(layout, stride);

    layout = temp;
}

void internal::darray_push(internal::memory_layout& layout, u64 stride, u64 index, void* valuePtr) {
    if (index != invalid_u64 && index >= layout.length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", layout.length, index);
        return;
    }

    if (index == invalid_u64) {
        index = layout.length;
    }

    if (layout.length >= layout.capacity) {
        u64 capacity = layout.capacity == 0 ? 1 : layout.capacity;    
        internal::darray_resize(layout, internal::resize_factor * capacity, stride);
    }

    u64 address = (u64)layout.block;

    if (layout.length != 0 && index != layout.length) {
        memory::fbcopy((void*)(address + ((index + 1) * stride)), (void*)((char*)layout.block + (index * stride)), stride * (layout.length - index));
    }

    void* dest = (void*)(address + (index * stride));

     memory::fbcopy(dest, valuePtr, stride);

     layout.length++;
}

void internal::darray_pop(internal::memory_layout& layout, u64 stride, u64 index) {
    if (index != invalid_u64 && index >= layout.length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", layout.length, index);
        return;
    }

    if (index == invalid_u64) {
        index = layout.length - 1;
    }

    u64 address = (u64)layout.block;
    if (index != layout.length - 1) {
        memory::fbcopy((void*)(address + (index * stride)), (void*)(address + ((index + 1) * stride)), stride * (layout.length - index));
    }

    layout.length--;
}