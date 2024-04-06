#pragma once

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using f32 = float;
using f64 = double;

using b8 = bool;

#ifdef _MSC_VER
#define STATIC_ASSERT static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

STATIC_ASSERT(sizeof(u8)  == 1, "Expected u8 to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

STATIC_ASSERT(sizeof(i8)  == 1, "Expected i8 to be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");

STATIC_ASSERT(sizeof(b8)  == 1, "Expected b8 to be 1 byte");

static constexpr u64 invalid_u64 = -1ULL;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define FBPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "A 64-bit architecture is required."
#endif
#else
#error "Unsupported platform. Fabric only runs on Windows."
#endif

#ifdef FBEXPORT
#ifdef _MSC_VER
#define FB_API _declspec(dllexport)
#else
#define FB __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define FB_API _declspec(dllimport)
#else
#define FB_API
#endif
#endif
