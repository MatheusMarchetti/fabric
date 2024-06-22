#pragma once

#include "defines.hpp"

namespace ftl {
    namespace internal {
        struct memory_layout {
            u64 capacity = 0;
            u64 length = 0;
            void* block = nullptr;
        };

        void darray_create(memory_layout& layout, u64 capacity, u64 stride);
        void darray_destroy(memory_layout& layout, u64 stride);
        void darray_copy(memory_layout& src, memory_layout& dst, u64 stride);

        void darray_resize(memory_layout& layout, u64 newSize, u64 stride);
        void darray_push(memory_layout& layout, u64 stride, u64 index, void* valuePtr);
        void darray_pop(memory_layout& layout, u64 stride, u64 index);

        static constexpr u64 resize_factor = 2;
    }  // namespace internal

    template <typename T>
    class FBAPI darray {
       public:
        darray() = default;

        darray(u64 capacity) {
            internal::darray_create(memory, capacity, sizeof(T));
        }

        ~darray() {
            internal::darray_destroy(memory, sizeof(T));
        }

        darray(const darray<T>& other) {
            internal::darray_copy(other.memory, memory, sizeof(T));
        }

        darray<T>& operator=(const darray<T>& other) {
            internal::darray_copy(other.memory, memory, sizeof(T));

            return *this;
        }

        T& operator[](u64 index) {
            T* block = (T*)memory.block;
            return block[index];
        }

        u64 capacity() const {
            return memory.capacity;
        }

        u64 length() const {
            return memory.length;
        }

        u64 stride() const { return sizeof(T); }

        b8 empty() const {
            return memory.length == 0;
        }

        void reserve(u64 newCapacity) {
            internal::darray_create(memory, newCapacity, sizeof(T));
        }

        void resize(u64 newSize) {
            internal::darray_resize(memory, newSize, sizeof(T));
        }

        T& push(const T& value, u64 index = end_of_array) {
            internal::darray_push(memory, sizeof(T), index, (void*)(&value));
            T* block = (T*)memory.block;
            return block[index != end_of_array ? index : length() - 1];
        }

        void pop(u64 index = end_of_array) {
            internal::darray_pop(memory, sizeof(T), index);
        }

        void clear() {
            memory.length = 0;
        }

        T* data() { return (T*)memory.block; }

       private:
        internal::memory_layout memory;

       private:
        static constexpr u64 end_of_array = invalid_u64;
    };
}  // namespace ftl