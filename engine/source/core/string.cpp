#include "core/string.hpp"
#include "core/memory.hpp"

#include <string.h>

using namespace fabric;

ftl::string::string(u64 size) {
    _size = size;

    if (size > 0) {
        _buffer = (char*)memory::fballocate(size, memory::MEMORY_TAG_STRING);
    }
}

ftl::string::string(char* str) {
    u64 length = this->length(str);
    resize(length + 1);

    memory::fbcopy((void*)_buffer, str, length);
    _buffer[length] = 0;
}

ftl::string::string(const char* str) {
    u64 length = this->length(str);
    resize(length + 1);

    memory::fbcopy((void*)_buffer, str, length);
    _buffer[length] = 0;
}

ftl::string::~string() {
    if(_size > 0) {
        memory::fbfree((void*)_buffer, _size, memory::MEMORY_TAG_STRING);
    }
}

ftl::string::string(const ftl::string& other) {
    *this = other;
}

ftl::string& ftl::string::operator=(const ftl::string& other) {
    resize(other._size);

    memory::fbcopy((void*)_buffer, other._buffer, other._size);

    return *this;
}

ftl::string& ftl::string::operator=(char* str) {
    u64 length = this->length(str);
    resize(length + 1);

    memory::fbcopy((void*)_buffer, str, length);
    _buffer[length] = 0;

    return *this;
}

void ftl::string::resize(u64 new_size) {
    if (_size < new_size) {
        if (_size != 0) {
            memory::fbfree((void*)_buffer, _size, memory::MEMORY_TAG_STRING);
        }
        _buffer = (char*)memory::fballocate(new_size, memory::MEMORY_TAG_STRING);
        _size = new_size;
    }
}

u64 ftl::string::length(char* ptr) const {
    return strlen(ptr);
}

u64 ftl::string::length(const char* ptr) const {
    return strlen(ptr);
}

b8 ftl::string::operator==(const ftl::string& other) {
    return strcmp(_buffer, other._buffer) == 0;
}

ftl::string& ftl::string::operator+=(const ftl::string& other) {
    if(_size == 0) {
        _buffer = (char*)memory::fballocate(other._size, memory::MEMORY_TAG_STRING);
        *this = other;
        
        return *this;
    };

    u64 total_length = other.length() + this->length();

    if(_size < total_length + 1) {
        char* temp = (char*)memory::fballocate(total_length + 1, memory::MEMORY_TAG_STRING);
        memory::fbcopy((void*)temp, _buffer, this->length());
        memory::fbcopy((void*)(temp + this->length()), other._buffer, other.length());

        memory::fbfree((void*)_buffer, _size, memory::MEMORY_TAG_STRING);
        _size = total_length + 1;
        _buffer = temp;
        _buffer[total_length] = 0;

        return *this;
    }

    memory::fbcopy((void*)(_buffer + this->length()), other._buffer, other.length());
    _buffer[total_length] = 0;

    return *this;
}

ftl::string& ftl::string::operator+=(const char* str) {
    return *this += ftl::string(str);
}