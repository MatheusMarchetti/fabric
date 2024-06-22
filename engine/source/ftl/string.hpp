#pragma once

#include "defines.hpp"

namespace fabric::ftl {
    class FBAPI string {
       public:
        string() = default;
        string(char* str);
        string(const char* str);
        ~string();
        string(const string& other);
        string& operator=(const string& other);

        u64 length() const;
        const char* data() const { return buffer; }

        b8 operator==(const string& other);

        i32 format(const char* format, ...);

       private:
        i32 format_v(const char* format, void* va_listp);

       private:
        char* buffer = nullptr;
    };
}  // namespace fabric::ftl