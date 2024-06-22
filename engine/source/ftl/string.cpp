#include "ftl/string.hpp"
#include "core/memory.hpp"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

using namespace fabric;
using namespace ftl;

string::string(char* str) {
    u64 length = strlen(str);

    buffer = (char*)memory::fballocate(length + 1, memory::MEMORY_TAG_STRING);
    memory::fbcopy(buffer, str, length);
    buffer[length] = 0;
}

string::string(const char* str) {
    u64 length = strlen(str);

    buffer = (char*)memory::fballocate(length + 1, memory::MEMORY_TAG_STRING);
    memory::fbcopy(buffer, str, length);
    buffer[length] = 0;
}

string::~string() {
    u64 length = strlen(buffer);

    memory::fbfree(buffer, length + 1, memory::MEMORY_TAG_STRING);
}

string::string(const string& other) {
    if (buffer) {
        u64 length = strlen(buffer);
        memory::fbfree(buffer, length + 1, memory::MEMORY_TAG_STRING);
    }

    u64 length = strlen(other.buffer);

    buffer = (char*)memory::fballocate(length + 1, memory::MEMORY_TAG_STRING);
    memory::fbcopy(buffer, other.buffer, length);
}

string& string::operator=(const string& other) {
    if (buffer) {
        u64 length = strlen(buffer);
        memory::fbfree(buffer, length + 1, memory::MEMORY_TAG_STRING);
    }

    u64 length = strlen(other.buffer);

    buffer = (char*)memory::fballocate(length + 1, memory::MEMORY_TAG_STRING);
    memory::fbcopy(buffer, other.buffer, length);

    return *this;
}

b8 string::operator==(const string& other) {
    return strcmp(buffer, other.buffer) == 0;
}

i32 string::format(const char* format, ...) {
    if (buffer) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = format_v(format, arg_ptr);
        va_end(arg_ptr);

        return written;
    }

    return -1;
}

i32 string::format_v(const char* format, void* va_listp) {
    if(buffer) {
        char temp[32000];
        i32 written = vsnprintf(temp, 32000, format, (va_list)va_listp);
        temp[written] = 0;

        u64 length = strlen(buffer);

        if(length < written) {
            memory::fbfree(buffer, length + 1, memory::MEMORY_TAG_STRING);
            buffer = (char*)memory::fballocate(written + 1, memory::MEMORY_TAG_STRING);
        }

        memory::fbcopy(buffer, temp, written + 1);

        return written;
    }

    return -1;
}