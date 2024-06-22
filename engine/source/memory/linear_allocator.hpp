#pragma once

#include "defines.hpp"

namespace fabric::memory {
    class FBAPI linear_allocator {
        public:
        linear_allocator() = default;
        ~linear_allocator();

        void create(u64 size, void* memory = nullptr);
        void destroy(void* memory = nullptr);

        void* allocate(u64 size);
        void reset();

        private:
        void* memory = nullptr;
        u64 total_size = 0;
        u64 allocated = 0;
    };
}