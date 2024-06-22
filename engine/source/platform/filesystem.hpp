#pragma once

#include "defines.hpp"

namespace fabric::filesystem {
    enum file_modes : u8 {
        FILE_MODE_READ = 0x1,
        FILE_MODE_WRITE = 0x2
    };

    class FBAPI file {
       public:
        file() = default;
        ~file() = default;

        b8 exists(const char* path);
        b8 open(const char* path, file_modes mode, b8 binary);
        void close();

        b8 is_valid() { return handle != nullptr; }

        b8 read_line(char** buffer);
        b8 write_line(const char* text);

        b8 read(void* data, u64* bytesRead, u64 dataSize);
        b8 read(u8** data, u64* bytesRead);
        b8 write(const void* data, u64* bytesWritten, u64 dataSize);

       private:
        void* handle = nullptr;
    };
}  // namespace fabric::filesystem