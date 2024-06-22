#include "core/logger.hpp"
#include "core/asserts.hpp"
#include "core/memory.hpp"
#include "platform/platform.hpp"

// TODO: Temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

using namespace fabric;

namespace {
    struct system_state {
        b8 initialized = false;  // TODO: Remove, just need something in here temporarily
    };

    static system_state* state;
}  // namespace

b8 logger::initialize(u64& memory_requirement, void* memory) {
    memory_requirement = sizeof(system_state);
    if(!memory) {
        return true;
    }

    if (state) {
        FBERROR("Logger system was already initialized!");
        return false;
    }
    state = (system_state*)memory;
    memory::fbzero(state, sizeof(system_state));

    // TODO: Create log file

    // NOTE: Eventually the logger will run on a different (fail-safe) process. This process will be launched from here.

    state->initialized = true;

    FBINFO("Logger system initialized.");

    return true;
}

void logger::terminate() {
    // TODO: Cleanup logging/write queued entries.

    // NOTE: Eventually the logger will run on a different (fail-safe) process. This process will be shutdown from here.
    state = nullptr;
    
}

void logger::report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    logger::log_output(logger::LOG_LEVEL_FATAL, "Assertion failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}

void logger::log_output(logger::log_level level, const char* message, ...) {
    // NOTE: Even if the logger system fails to initialize for some reason, this should still happen on the debug console.
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

    if (is_error) {
        platform::console_write_error(final_message, level);
    } else {
        platform::console_write(final_message, level);
    }
}