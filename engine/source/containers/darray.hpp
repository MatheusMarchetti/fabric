#pragma once

#include "defines.hpp"

namespace ftl {
    namespace internal {
        void* darray_create(u64 capacity, u64 stride);
        void darray_destroy(void* memory, u64 stride);
        void* darray_copy(void* original, u64 stride);

        void* darray_resize(void* current, u64 newSize, u64 stride);
        void* darray_push(void* current, u64 stride, u64 index, void* valuePtr);
        void darray_pop(void* current, u64 stride, u64 index);
    }  // namespace internal

    template <typename T>
    class FBAPI darray {
       public:
        darray() : elements(nullptr) {}

        darray(u64 capacity) {
            elements = (T*)internal::darray_create(capacity, sizeof(T));
        }

        ~darray() {
            if (elements) {
                internal::darray_destroy(elements, sizeof(T));
            }
        }

        darray(const darray<T>& other) {
            if (other.elements) {
                this->elements = (T*)internal::darray_copy(other.elements, sizeof(T));
            }
        }

        darray<T>& operator=(const darray<T>& other) {
            if (other.elements) {
                this->elements = (T*)internal::darray_copy(other.elements, sizeof(T));
            }

            return *this;
        }

        T& operator[](u64 index) {
            return elements[index];
        }

        u64 capacity() const {
            if (!elements) {
                return 0;
            }
            return *(u64*)((char*)elements - 2 * sizeof(u64));
        }

        u64 length() const {
            if (!elements) {
                return 0;
            }
            return *(u64*)((char*)elements - 1 * sizeof(u64));
        }

        u64 stride() const { return sizeof(T); }

        b8 empty() const {
            return (!elements || length() == 0);
        }

        void reserve(u64 newCapacity) {
            elements = (T*)internal::darray_create(newCapacity, sizeof(T));
        }

        void resize(u64 newSize) {
            elements = (T*)internal::darray_resize(elements, newSize, sizeof(T));
        }

        T& push(const T& value, u64 index = end_of_array) {
            if (!elements) {
                reserve(1);
            }

            elements = (T*)internal::darray_push(elements, sizeof(T), index, (void*)(&value));
            return elements[index != end_of_array ? index : length() - 1];
        }

        void pop(u64 index = end_of_array) {
            internal::darray_pop(elements, sizeof(T), index);
        }

        void clear() const {
            if (!elements) {
                return;
            }
            u64* length = (u64*)((char*)elements - 1 * sizeof(u64));
            *length = 0;
        }

        T* data() { return elements; }

       private:
           T* elements;

       private:
        static constexpr u64 end_of_array = invalid_u64;
        static constexpr u64 default_capacity = 1;
    };
}  // namespace fabric::ftl