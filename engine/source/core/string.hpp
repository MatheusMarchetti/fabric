#pragma once

#include "defines.hpp"

namespace fabric::ftl {
    class FB_API string {
       public:
        string(u64 size = 0);
        string(char* str);
        string(const char* str);
        ~string();
        string(const string& other);
        string& operator=(const string& other);
        string& operator=(char* str);

        u64 length() const { return length(_buffer); }
        u64 size() { return _size; }
        const char* str() const { return _buffer; }

        b8 operator==(const string& other);
        string& operator+=(const string& other);
        string& operator+=(const char* str);

       private:
        void resize(u64 new_size);
        u64 length(char* ptr) const;
        u64 length(const char* ptr) const;

       private:
        char* _buffer = nullptr;
        u64 _size = 0;
    };
}  // namespace fabric::ftl