#pragma once

#include "defines.hpp"

// Disable assertions by commenting out this line
#define FB_ASSERTIONS_ENABLED

#ifdef FB_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugbreak() __debugbreak()
#else
#define debugbreak() __builtin_trap()
#endif

namespace fabric::logger {
    extern "C" {
    FB_API void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);
    }
}

#define FBASSERT(expr)                                                               \
    {                                                                                \
        if (expr) {                                                                  \
        } else {                                                                     \
            fabric::logger::report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugbreak();                                                            \
        }                                                                            \
    }

#define FBASSERT_MSG(expr, message)                                                       \
    {                                                                                     \
        if (expr) {                                                                       \
        } else {                                                                          \
            fabric::logger::report_assertion_failure(#expr, message, __FILE__, __LINE__); \
            debugbreak();                                                                 \
        }                                                                                 \
    }

#ifdef _DEBUG
#define FBASSERT_DEBUG(expr)                                                         \
    {                                                                                \
        if (expr) {                                                                  \
        } else {                                                                     \
            fabric::logger::report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugbreak();                                                            \
        }                                                                            \
    }
#else
#define FBASSERT_DEBUG(expr)
#endif
#else
#define FBASSERT(expr)
#define FBASSERT_MSG(expr, message)
#define FBASSERT_DEBUG(expr)
#endif