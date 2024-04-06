#pragma once

#include "defines.hpp"

namespace fabric::memory {
    enum memory_tag {
        MEMORY_TAG_UNKNOWN = 0,
        MEMORY_TAG_ARRAY,
        MEMORY_TAG_DARRAY,
        MEMORY_TAG_DICT,
        MEMORY_TAG_RING_QUEUE,
        MEMORY_TAG_BST,
        MEMORY_TAG_STRING,
        MEMORY_TAG_APPLICATION,
        MEMORY_TAG_JOB,
        MEMORY_TAG_TEXTURE,
        MEMORY_TAG_MATERIAL_INSTANCE,
        MEMORY_TAG_RENDERER,
        MEMORY_TAG_SCENE,
        MEMORY_TAG_COMPONENT,
        MEMORY_TAG_OPERATOR_NEW,
        MEMORY_TAG_COUNT
    };

    b8 initialize();
    void terminate();

    FB_API void* fballocate(u64 size, memory_tag tag);
    FB_API void fbfree(void* block, u64 size, memory_tag tag);
    FB_API void* fbzero(void* block, u64 size);
    FB_API void* fbcopy(void* dst, const void* src, u64 size);
    FB_API void* fbset(void* dst, i32 value, u64 size);

    FB_API void log_memory_usage();
}  // namespace fabric::memory