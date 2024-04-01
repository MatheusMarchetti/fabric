#include "core/logger.hpp"
#include "core/asserts.hpp"
#include "platform/platform.hpp"

// TODO: Temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

using namespace fabric;

b8 logger::initialize() {
    // TODO: Create log file

    // NOTE: Eventually the logger will run on a different (fail-safe) process. This process will be launched from here.

    return true;
}

void logger::shutdown() {
    // TODO: Cleanup logging/write queued entries.

    // NOTE: Eventually the logger will run on a different (fail-safe) process. This process will be shutdown from here.
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(logger::LOG_LEVEL_FATAL, "Assertion failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

void log_output(logger::log_level level, const char* message, ...) {
    static const char* level_string[logger::LOG_LEVEL_COUNT] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: "};

    b8 is_error = level < logger::LOG_LEVEL_WARN;

    constexpr i32 message_length = 32000;
    char out_message[message_length];
    memset(out_message, 0, sizeof(out_message));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, message_length, message, arg_ptr);
    va_end(arg_ptr);

    char final_message[message_length];
    sprintf(final_message, "%s%s\n", level_string[level], out_message);

    if(is_error) {
        platform::console_write_error(final_message, level);
    } else {
        platform::console_write(final_message, level);
    }
}