#include "platform/filesystem.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace fabric;

b8 filesystem::file::open(const char* path, file_modes mode, b8 binary) {
    const char* modeStr;
    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "w+b" : "w+";
    } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        modeStr = binary ? "rb" : "r";
    } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "wb" : "w";
    } else {
        FBERROR("Invalid mode passed while trying to open file %s", path);
        return false;
    }

    FILE* file = fopen(path, modeStr);
    if (!file) {
        FBERROR("Error opening file %s", path);
        return false;
    }
    handle = file;

    return true;
}

void filesystem::file::close() {
    if (handle) {
        fclose((FILE*)handle);
        handle = nullptr;
    }
}

b8 filesystem::file::read_line(char** buffer) {
    if (handle) {
        char buf[32000];
        if (fgets(buf, 32000, (FILE*)handle) != 0) {
            u64 length = strlen(buf);
            *buffer = (char*)memory::fballocate((sizeof(char) * length) + 1, memory::MEMORY_TAG_STRING);
            strcpy(*buffer, buf);
            return true;
        }
    }

    return false;
}

b8 filesystem::file::write_line(const char* text) {
    if (handle) {
        i32 result = fputs(text, (FILE*)handle);
        if (result != EOF) {
            result = fputc('\n', (FILE*)handle);
        }

        fflush((FILE*)handle);
        return result != EOF;
    }

    return false;
}

b8 filesystem::file::read(void* data, u64* bytesRead, u64 dataSize) {
    if (handle && data) {
        *bytesRead = fread(data, 1, dataSize, (FILE*)handle);
        if (*bytesRead != dataSize) {
            return false;
        }
        return true;
    }
    return false;
}

b8 filesystem::file::read(u8** data, u64* bytesRead) {
    if (handle) {
        fseek((FILE*)handle, 0, SEEK_END);
        u64 size = ftell((FILE*)handle);
        rewind((FILE*)handle);

        *data = (u8*)memory::fballocate(sizeof(u8) * size, memory::MEMORY_TAG_STRING);
        *bytesRead = fread(*data, 1, size, (FILE*)handle);
        if (*bytesRead != size) {
            return false;
        }
        return true;
    }
    return false;
}

b8 filesystem::file::write(const void* data, u64* bytesWritten, u64 dataSize) {
    if (handle) {
        *bytesWritten = fwrite(data, 1, dataSize, (FILE*)handle);
        if (*bytesWritten != dataSize) {
            return false;
        }
        fflush((FILE*)handle);
        return true;
    }
    return false;
}