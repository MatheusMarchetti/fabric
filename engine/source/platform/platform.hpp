#pragma once

#include "defines.hpp"

namespace fabric::platform {
    struct window {
        const char* name;
        i16 x;
        i16 y;
        u16 width;
        u16 height;
        void* internal_state;
    };
    b8 initialize(fabric::platform::window& platformState);
    void terminate(fabric::platform::window& platformState);

    b8 update(fabric::platform::window& platformState);

    void* allocate_memory(u64 size, b8 aligned);
    void free_memory(void* block, b8 aligned);
    void* zero_memory(void* block, u64 size);
    void* copy_memory(void* dest, const void* source, u64 size);
    void* set_memory(void* dest, i32 value, u64 size);

    void console_write(const char* message, u8 color);
    void console_write_error(const char* message, u8 color);

    f64 get_absolute_time();

    void sleep(u64 ms);
}  // namespace fabric::platform