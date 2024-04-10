#include "containers/darray.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

using namespace fabric;
using namespace ftl;

namespace {
    static constexpr u64 header_size = internal::memory_layout::header_size;
    static constexpr u64 resize_factor = 2;
}  // namespace

internal::memory_layout* internal::darray_create(u64 capacity, u64 stride) {
    u64 array_size = capacity * stride;
    internal::memory_layout* new_array = (internal::memory_layout*)memory::fballocate(header_size + array_size, memory::MEMORY_TAG_DARRAY);
    u64* elements_array = (u64*)new_array;
    elements_array += header_size / sizeof(u64);
    new_array->capacity = capacity;
    new_array->length = 0;
    new_array->elements = (void*)(elements_array);

    return new_array;
}

void internal::darray_destroy(internal::memory_layout* memory, u64 stride) {
    u64 total_size = header_size + memory->capacity * stride;
    memory::fbfree(memory, total_size, memory::MEMORY_TAG_DARRAY);
}

internal::memory_layout* internal::darray_copy(internal::memory_layout* original, u64 stride) {
    internal::memory_layout* new_array = internal::darray_create(original->capacity, stride);
    new_array->capacity = original->capacity;
    new_array->length = original->length;
    new_array->elements = memory::fbcopy(new_array->elements, original->elements, original->length * stride);

    return new_array;
}

internal::memory_layout* internal::darray_resize(internal::memory_layout* current, u64 stride) {
    u64 length = current->length;
    u64 capacity = current->capacity;

    internal::memory_layout* temp = internal::darray_create(resize_factor * capacity, stride);
    memory::fbcopy(temp, current, length * stride);
    temp->length = length;

    internal::darray_destroy(current, stride);
    return temp;
}

void* internal::darray_push(internal::memory_layout* current, u64 stride, u64 index, void* valuePtr) {
    if (index != invalid_u64 && index >= current->length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", current->length, index);
        return nullptr;
    }

    if (index == invalid_u64) {
        index = current->length == 0 ? current->length : current->length - 1;
    }

    if (current->length >= current->capacity) {
        current = internal::darray_resize(current, stride);
    }

    u64 address = (u64)current->elements;

    if (current->length != 0 && index != current->length - 1) {
        memory::fbcopy((void*)(address + ((index + 1) * stride)), (void*)(address + (index * stride)), stride * (current->length - index));
    }

    current->elements = memory::fbcopy((void*)(address + (index * stride)), valuePtr, stride);
    current->length++;

    return valuePtr;
}

void internal::darray_pop(internal::memory_layout* current, u64 stride, u64 index) {
    if (index != invalid_u64 && index >= current->length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", current->length, index);
        return;
    }

    u64 address = (u64)current->elements;
    if (index != current->length - 1) {
        memory::fbcopy((void*)(address + (index * stride)), (void*)(address + ((index + 1) * stride)), stride * (current->length - index));
    }

    current->length--;
}