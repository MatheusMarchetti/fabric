#include "memory/linear_allocator.hpp"
#include "core/memory.hpp"
#include "core/logger.hpp"

using namespace fabric;

memory::linear_allocator::~linear_allocator() {
    if(memory) {
        FBWARN("Destroying linear_allocator instance caused a memory leak! Remember to call linear_allocator::destroy and free the memory (if allocated externally).");
    }
}

void memory::linear_allocator::create(u64 size, void* memory) {
    total_size = size;
    allocated = 0;
    if(memory) {
        this->memory = memory;
    } else {
        this->memory = fballocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
}

void memory::linear_allocator::destroy(void* memory) {
    if(memory) {
        this->memory = nullptr;
    } else {
        fbfree(this->memory, total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
        this->memory = nullptr;
    }

    total_size = 0;
    allocated = 0;
}

void* memory::linear_allocator::allocate(u64 size) {
    if(memory) {
        if(allocated + size > total_size) {
            u64 remaining = total_size - allocated;
            FBERROR("linear_allocator::allocate: Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return nullptr;
        }

        void* block = (void*)((char*)memory + allocated);
        allocated += size;

        return block;
    }

    FBERROR("linear_allocator::allocate: Tried to allocate before calling linear_allocator::create:");
    return nullptr;
}

void memory::linear_allocator::reset() {
    if(memory) {
        allocated = 0;
        fbzero(memory, total_size);
    }
}