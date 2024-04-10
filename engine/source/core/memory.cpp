#include "core/memory.hpp"
#include "core/logger.hpp"
#include "platform/platform.hpp"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

using namespace fabric;

namespace {
    struct memory_stats {
        u64 total_allocated;
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
        "JOB        ",
        "TEXTURE    ",
        "MAT_INST   ",
        "RENDERER   ",
        "SCENE      ",
        "COMPONENT  ",
        "OP_NEW     "};

    static memory_stats statistics;

    b8 is_initialized = false;
}  // namespace

b8 memory::initialize() {
    if (is_initialized) {
        FBERROR("Memory system was already initialized!");
        return false;
    }
    platform::zero_memory(&statistics, sizeof(memory_stats));

    FBINFO("Memory system initialized.");

    return is_initialized = true;
}

void memory::terminate() {
    is_initialized = false;
}

void* memory::fballocate(u64 size, memory::memory_tag tag) {
    if (tag == memory::MEMORY_TAG_UNKNOWN) {
        FBWARN("fballocate called using MEMORY_TAG_UNKNOWN. It's recommended for all allocations to be properly categorized");
    }

    statistics.total_allocated += size;
    statistics.tagged_allocations[tag] += size;

    // TODO: Memory alignment
    void* block = platform::allocate_memory(size, false);
    platform::zero_memory(block, size);
    return block;
}

void memory::fbfree(void* block, u64 size, memory::memory_tag tag) {
    if (tag == memory::MEMORY_TAG_UNKNOWN) {
        FBWARN("fbfree called using MEMORY_TAG_UNKNOWN. It's recommended for all allocations to be properly categorized");
    }

    statistics.total_allocated -= size;
    statistics.tagged_allocations[tag] -= size;

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

        if (statistics.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = statistics.tagged_allocations[i] / (f32)gib;
        } else if (statistics.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = statistics.tagged_allocations[i] / (f32)mib;
        } else if (statistics.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = statistics.tagged_allocations[i] / (f32)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)statistics.tagged_allocations[i];
        }

        i32 length = snprintf(buffer + offset, buffer_size, "   %s: %.2f%s\n", memory_tag_string[i], amount, unit);
        offset += length;
    }

    FBINFO(buffer);
}

void* operator new(u64 size) {
    return memory::fballocate(size, memory::MEMORY_TAG_OPERATOR_NEW);
}

void operator delete(void* block) noexcept {
    // NOTE: _msize() is Windows specific. I couldn't find a better way to figure out the size of a memory block, since operator delete doesn't have this information.
    memory::fbfree(block, _msize(block), memory::MEMORY_TAG_OPERATOR_NEW);
}

void* operator new[](u64 size) {
    return memory::fballocate(size, memory::MEMORY_TAG_OPERATOR_NEW);
}

void operator delete[](void* block) noexcept {
    // NOTE: _msize() is Windows specific. I couldn't find a better way to figure out the size of a memory block, since operator delete doesn't have this information.
    memory::fbfree(block, _msize(block), memory::MEMORY_TAG_OPERATOR_NEW);
}