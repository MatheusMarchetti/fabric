#pragma once

#ifdef __cplusplus
#define SWIZZLE_VEC2(comp1, comp2) \
    struct {                       \
        T comp1;                   \
        T comp2;                   \
    };

#define ARITHMETIC_VEC2(op)                                             \
    vec2<T> operator op(const vec2<T>& other) const {                   \
        return vec2<T>(vec[0] op other.vec[0], vec[1] op other.vec[1]); \
    }

#define SWIZZLE_VEC3(comp1, comp2, comp3) \
    struct {                              \
        T comp1;                          \
        T comp2;                          \
        T comp3;                          \
    };                                    \
    struct {                              \
        vec2<T> comp1##comp2;             \
        T _##comp3;                       \
    };                                    \
    struct {                              \
        T _##comp1;                       \
        vec2<T> comp2##comp3;             \
    };

#define ARITHMETIC_VEC3(op)                                                                     \
    vec3<T> operator op(const vec3<T>& other) const {                                           \
        return vec3<T>(vec[0] op other.vec[0], vec[1] op other.vec[1], vec[2] op other.vec[2]); \
    }

#define SWIZZLE_VEC4(comp1, comp2, comp3, comp4) \
    struct {                                     \
        T comp1;                                 \
        T comp2;                                 \
        T comp3;                                 \
        T comp4;                                 \
    };                                           \
    struct {                                     \
        vec2<T> comp1##comp2;                    \
        vec2<T> comp3##comp4;                    \
    };                                           \
    struct {                                     \
        T _##comp1;                              \
        vec2<T> comp2##comp3;                    \
        T _##comp4;                              \
    };                                           \
    struct {                                     \
        vec3<T> comp1##comp2##comp3;             \
        T __##comp4;                             \
    };                                           \
    struct {                                     \
        T __##comp1;                             \
        vec3<T> comp2##comp3##comp4;             \
    };

#define ARITHMETIC_VEC4(op)                                                                                             \
    vec4<T> operator op(const vec4<T>& other) const {                                                                   \
        return vec4<T>(vec[0] op other.vec[0], vec[1] op other.vec[1], vec[2] op other.vec[2], vec[3] op other.vec[3]); \
    }

template <typename T>
struct vec2 {
    vec2(T val = 0) : vec{val, val} {}
    vec2(T a, T b) : vec{a, b} {}

    ARITHMETIC_VEC2(+);
    ARITHMETIC_VEC2(-);
    ARITHMETIC_VEC2(*);
    ARITHMETIC_VEC2(/);

    vec2<T> operator*(T scalar) {
        return vec2<T>(vec[0] * scalar, vec[1] * scalar);
    }

    vec2<T> operator/(T scalar) {
        return vec2<T>(vec[0] / scalar, vec[1] / scalar);
    }

    T length_sq() { vec[0] * vec[0] + vec[1] * vec[1]; }

    f32 length() { return fbsqrt(length_sq()); }

    void normalize() {
        f32 len = length();
        vec[0] /= len;
        vec[1] /= len;
    }

    union {
        T vec[2];
        SWIZZLE_VEC2(x, y);
        SWIZZLE_VEC2(u, v);
        SWIZZLE_VEC2(w, h);
    };
};

template <typename T>
struct vec3 {
    vec3(T val = 0) : vec{val, val, val} {}
    vec3(T a, T b, T c) : vec{a, b, c} {}
    vec3(const vec2<T>& vec0, T c) : vec{vec0.vec[0], vec0.vec[1], c} {}
    vec3(T a, const vec2<T>& vec0) : vec{a, vec0.vec[0], vec0.vec[1]} {}

    ARITHMETIC_VEC3(+);
    ARITHMETIC_VEC3(-);
    ARITHMETIC_VEC3(*);
    ARITHMETIC_VEC3(/);

    vec3<T> operator*(T scalar) {
        return vec3<T>(vec[0] * scalar, vec[1] * scalar, vec[2] * scalar);
    }

    vec3<T> operator/(T scalar) {
        return vec3<T>(vec[0] / scalar, vec[1] / scalar, vec[2] / scalar);
    }

    T length_sq() { return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]; }

    f32 length() { return fbsqrt(length_sq()); }

    void normalize() {
        f32 len = length();
        vec[0] /= len;
        vec[1] /= len;
        vec[2] /= len;
    }

    union {
        T vec[3];
        SWIZZLE_VEC3(x, y, z);
        SWIZZLE_VEC3(r, g, b);
        SWIZZLE_VEC3(u, v, w);
    };
};

template <typename T>
struct vec4 {
    vec4(T val = 0) : vec{val, val, val, val} {}
    vec4(T a, T b, T c, T d) : vec{a, b, c, d} {}
    vec4(const vec2<T>& vec0, const vec2<T>& vec1) : vec{vec0.vec[0], vec0.vec[1], vec1.vec[0], vec1.vec[1]} {}
    vec4(T a, const vec2<T>& vec0, T d) : vec{a, vec0.vec[0], vec0.vec[1], d} {}
    vec4(const vec3<T>& vec0, T d) : vec{vec0.vec[0], vec0.vec[1], vec0.vec[2], d} {}
    vec4(T a, const vec3<T>& vec1) : vec{a, vec1.vec[0], vec1.vec[1], vec1.vec[2]} {}

    ARITHMETIC_VEC4(+);
    ARITHMETIC_VEC4(-);
    ARITHMETIC_VEC4(*);
    ARITHMETIC_VEC4(/);

    vec4<T> operator*(T scalar) {
        return vec4<T>(vec[0] * scalar, vec[1] * scalar, vec[2] * scalar, vec[3] * scalar);
    }

    vec4<T> operator/(T scalar) {
        return vec4<T>(vec[0] / scalar, vec[1] / scalar, vec[2] / scalar, vec[3] / scalar);
    }

    T length_sq() { vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]; }

    f32 length() { return fbsqrt(length_sq()); }

    void normalize() {
        f32 len = length();
        vec[0] /= len;
        vec[1] /= len;
        vec[2] /= len;
        vec[3] /= len;
    }

    union {
        T vec[4];
        SWIZZLE_VEC4(x, y, z, w);
        SWIZZLE_VEC4(r, g, b, a);
    };
};

using vec2f = vec2<f32>;
using vec2i = vec2<i32>;
using vec2u = vec2<u32>;

using vec3f = vec3<f32>;
using vec3i = vec3<i32>;
using vec3u = vec3<u32>;

using vec4f = vec4<f32>;
using vec4i = vec4<i32>;
using vec4u = vec4<u32>;
#else
using vec2f = float2;
using vec2i = int2;
using vec2u = uint2;

using vec3f = float3;
using vec3i = int3;
using vec3u = uint3;

using vec4f = float4;
using vec4i = int4;
using vec4u = uint4;
#endif