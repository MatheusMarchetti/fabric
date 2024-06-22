#include "core/memory.hpp"
#include "core/logger.hpp"
#include "platform/platform.hpp"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

using namespace fabric;

namespace {
    struct system_state {
        u64 total_allocated = 0;
        u64 allocation_count = 0;
        u64 tagged_allocations[memory::MEMORY_TAG_COUNT];
    };

    static const char* memory_tag_string[memory::MEMORY_TAG_COUNT] = {
        "UNKNOWN    ",
        "ARRAY      ",
        "DARRAY     ",
        "DICT       ",
        "RING_QUEUE ",
        "BST        ",
        "STRING     ",
        "APPLICATION",
        "LINEAR_ALLC",
        "JOB        ",
        "TEXTURE    ",
        "MAT_INST   ",
        "RENDERER   ",
        "SCENE      ",
        "COMPONENT  "};

    static system_state* state;
}  // namespace

b8 memory::initialize(u64& memory_requirement, void* memory) {
    memory_requirement = sizeof(system_state);
    if (!memory) {
        return true;
    }

    if (state) {
        FBERROR("Memory system was already initialized!");
        return false;
    }
    state = (system_state*)memory;
    memory::fbzero(state, sizeof(system_state));

    FBINFO("Memory system initialized.");

    return true;
}

void memory::terminate() {
    state = nullptr;
}

void* memory::fballocate(u64 size, memory::memory_tag tag) {
    if (state) {
        if (tag == memory::MEMORY_TAG_UNKNOWN) {
            FBWARN("memory::fballocate: Called using MEMORY_TAG_UNKNOWN. It's recommended for all allocations to be properly categorized");
        }

        state->total_allocated += size;
        state->tagged_allocations[tag] += size;
        state->allocation_count++;
    }

    // TODO: Memory alignment
    void* block = platform::allocate_memory(size, false);
    platform::zero_memory(block, size);

    return block;
}

void memory::fbfree(void* block, u64 size, memory::memory_tag tag) {
    if(state) {
        if (tag == memory::MEMORY_TAG_UNKNOWN) {
            FBWARN("memory::fbfree: Called using MEMORY_TAG_UNKNOWN. It's recommended for all allocations to be properly categorized");
        }

        state->total_allocated -= size;
        state->tagged_allocations[tag] -= size;
    }

    // TODO: Memory alignment
    platform::free_memory(block, false);
}

void* memory::fbzero(void* block, u64 size) {
    return platform::zero_memory(block, size);
}

void* memory::fbcopy(void* dst, const void* src, u64 size) {
    return platform::copy_memory(dst, src, size);
}

void* memory::fbset(void* dst, i32 value, u64 size) {
    return platform::set_memory(dst, value, size);
}

void memory::log_memory_usage() {
    static constexpr u64 gib = 1024 * 1024 * 1024;
    static constexpr u64 mib = 1024 * 1024;
    static constexpr u64 kib = 1024;

    static constexpr i32 buffer_size = 8000;
    char buffer[buffer_size] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < memory::MEMORY_TAG_COUNT; i++) {
        char unit[4] = "XiB";
        f32 amount = 1.0f;

        if (state->tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = state->tagged_allocations[i] / (f32)gib;
        } else if (state->tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = state->tagged_allocations[i] / (f32)mib;
        } else if (state->tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = state->tagged_allocations[i] / (f32)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)state->tagged_allocations[i];
        }

        i32 length = snprintf(buffer + offset, buffer_size, "   %s: %.2f%s\n", memory_tag_string[i], amount, unit);
        offset += length;
    }

    FBINFO(buffer);
}

u64 memory::get_memory_allocation_count() {
    if (state) {
        return state->allocation_count;
    }

    return 0;
}

void* operator new(u64 size) {
    void* block = memory::fballocate(size + sizeof(u64), memory::MEMORY_TAG_UNKNOWN);
    u64* b = (u64*)block;
    *b = size;

    return (void*)(b + 1);
}

void operator delete(void* block) noexcept {
    void* b = (void*)((u64*)block - 1);
    u64 size = *((u64*)b);
    memory::fbfree(b, size, memory::MEMORY_TAG_UNKNOWN);
}

void* operator new[](u64 size) {
    void* block = memory::fballocate(size + sizeof(u64), memory::MEMORY_TAG_UNKNOWN);
    u64* b = (u64*)block;
    *b = size;

    return (void*)(b + 1);
}

void operator delete[](void* block) noexcept {
    void* b = (void*)((u64*)block - 1);
    u64 size = *((u64*)b);
    memory::fbfree(b, size, memory::MEMORY_TAG_UNKNOWN);
}