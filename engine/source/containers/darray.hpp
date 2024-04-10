#pragma once

#include "defines.hpp"

namespace fabric::ftl {
    namespace internal {
        struct memory_layout {
            u64 capacity;
            u64 length;
            void* elements;

            static constexpr u64 header_size = 2 * sizeof(u64);
        };

        memory_layout* darray_create(u64 capacity, u64 stride);
        void darray_destroy(memory_layout* memory, u64 stride);
        memory_layout* darray_copy(memory_layout* original, u64 stride);

        memory_layout* darray_resize(memory_layout* current, u64 stride);
        void* darray_push(memory_layout* current, u64 stride, u64 index, void* valuePtr);
        void darray_pop(memory_layout* current, u64 stride, u64 index);
    }  // namespace internal

    template <typename T>
    class FB_API darray {
       public:
        darray(u64 capacity = default_capacity) {
            memory = internal::darray_create(capacity, sizeof(T));
        }

        ~darray() {
            internal::darray_destroy(memory, sizeof(T));
        }

        darray(const darray<T>& other) {
            this->memory = internal::darray_copy(other.memory, sizeof(T));
        }

        darray<T>& operator=(const darray<T>& other) {
            this->memory = internal::darray_copy(other.memory, sizeof(T));

            return *this;
        }

        u64 capacity() const { return memory->capacity; }
        u64 length() const { return memory->length; }
        u64 stride() const { return sizeof(T); }
        b8 empty() const { return memory->length == 0; }

        void reserve(u64 newCapacity) {
            memory = internal::darray_create(newCapacity, sizeof(T));
        }

        void resize() {
            memory = internal::darray_resize(memory, sizeof(T));
        }

        const T& push(const T& value, u64 index = end_of_array) {
            return *(T*)internal::darray_push(memory, sizeof(T), index, (void*)(&value));
        }

        void pop(u64 index = end_of_array) {
            internal::darray_pop(memory, sizeof(T), index);
        }

        void clear() const { memory->length = 0; }

        T& operator[](u64 index) {
            T* elements = (T*)memory->elements;
            return elements[index];
        }

       private:
        internal::memory_layout* memory;

       private:
        static constexpr u64 end_of_array = invalid_u64;
        static constexpr u64 default_capacity = 1;
    };
}  // namespace fabric::ftl