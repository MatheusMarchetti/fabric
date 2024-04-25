#include "containers/darray.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

using namespace fabric;
using namespace ftl;

namespace {
    struct header {
        u64 capacity;
        u64 length;

        static constexpr u64 header_size = 2 * sizeof(u64);
    };

    header* get_header(void* block) {
        return (header*)((char*)block - header::header_size);
    }

    static constexpr u64 resize_factor = 2;
}  // namespace

void* internal::darray_create(u64 capacity, u64 stride) {
    u64 array_size = capacity * stride;
    header h = {
        .capacity = capacity,
        .length = 0
    };

    void* new_array = memory::fballocate(header::header_size + array_size, memory::MEMORY_TAG_DARRAY);

    memory::fbcopy(new_array, &h, header::header_size);

    return (void*)((char*)new_array + header::header_size);
}

void internal::darray_destroy(void* memory, u64 stride) {
    header* h = get_header(memory);
    u64 total_size = header::header_size + h->capacity * stride;
    memory::fbfree((void*)h, total_size, memory::MEMORY_TAG_DARRAY);
}

void* internal::darray_copy(void* original, u64 stride) {
    header* original_header = get_header(original);
    void* new_array = internal::darray_create(original_header->capacity, stride);

    header* new_header = get_header(new_array);
    new_header->capacity = original_header->capacity;
    new_header->length = original_header->length;

    memory::fbcopy(new_array, original, original_header->length * stride);

    return new_array;
}

void* internal::darray_resize(void* current, u64 newSize, u64 stride) {
    header* current_header = get_header(current);
    u64 length = current_header->length;

    void* temp = internal::darray_create(newSize, stride);

    header* temp_header = get_header(temp);
    temp_header->length = length;

    memory::fbcopy(temp, current, length * stride);

    internal::darray_destroy(current, stride);
    
    return temp;
}

void* internal::darray_push(void* current, u64 stride, u64 index, void* valuePtr) {
    header* current_header = get_header(current);

    if (index != invalid_u64 && index >= current_header->length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", current_header->length, index);
        return nullptr;
    }

    if (index == invalid_u64) {
        index = current_header->length;
    }

    if (current_header->length >= current_header->capacity) {
        current = internal::darray_resize(current, resize_factor * current_header->capacity, stride);
        current_header = get_header(current);
    }

    u64 address = (u64)current;

    if (current_header->length != 0 && index != current_header->length) {
        memory::fbcopy((void*)(address + ((index + 1) * stride)), (void*)((char*)current + (index * stride)), stride * (current_header->length - index));
    }

    void* dest = (void*)(address + (index * stride));

    //current->elements = memory::fbcopy((void*)(address + (index * stride)), valuePtr, stride);
     memory::fbcopy(dest, valuePtr, stride);

     current_header->length++;

    return current;
}

void internal::darray_pop(void* current, u64 stride, u64 index) {
    header* current_header = get_header(current);
    if (index != invalid_u64 && index >= current_header->length) {
        FBERROR("Index outside of the bounds of this array! Length: %i, index: %i", current_header->length, index);
        return;
    }

    if (index == invalid_u64) {
        index = current_header->length - 1;
    }

    u64 address = (u64)current;
    if (index != current_header->length - 1) {
        memory::fbcopy((void*)(address + (index * stride)), (void*)(address + ((index + 1) * stride)), stride * (current_header->length - index));
    }

    current_header->length--;
}