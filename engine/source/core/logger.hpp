#pragma once

#include "defines.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1

#if FRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#endif

namespace fabric::logger {
    enum log_level {
        LOG_LEVEL_FATAL = 0,
        LOG_LEVEL_ERROR = 1,
        LOG_LEVEL_WARN = 2,
        LOG_LEVEL_INFO = 3,
        LOG_LEVEL_DEBUG = 4,
        LOG_LEVEL_COUNT
    };

    b8 initialize();
    void shutdown();
}  // namespace fabric::logger

extern "C" {
FB_API void log_output(fabric::logger::log_level level, const char* message, ...);
}

#define FBFATAL(message, ...) log_output(fabric::logger::LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#define FBERROR(message, ...) log_output(fabric::logger::LOG_LEVEL_ERROR, message, ##__VA_ARGS__);

#if LOG_WARN_ENABLED == 1
#define FBWARN(message, ...) log_output(fabric::logger::LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define FBWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define FBINFO(message, ...) log_output(fabric::logger::LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define FBINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define FBDEBUG(message, ...) log_output(fabric::logger::LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define FBDEBUG(message, ...)
#endif
