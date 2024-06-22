#pragma once

#include "defines.hpp"

#undef min
#undef max

// NOTE: Fabric uses a right handed coordinate system.
//       +X points to the right, +Y points into the screen and +Z points up.
//       Vectors and matrices are stored in row major order.

namespace ftl {
#define FB_PI 3.14159265358979323846f
#define FB_2PI 2.0f * FB_PI
#define FB_HALF_PI 0.5f * FB_PI
#define FB_QUARTER_PI 0.25f * FB_PI
#define FB_INV_PI 1.0f / FB_PI
#define FB_INV_2PI 1.0f / FB_2PI
#define FB_SQRT_2 1.41421356237309504880f
#define FB_SQRT_3 1.73205080756887729352f
#define FB_INV_SQRT_2 0.70710678118654752440f
#define FB_INV_SQRT_3 0.57735026918962576450f
#define FB_DEG2RAD_MULTIPLIER FB_PI / 180.0f
#define FB_RAD2DEG_MULTIPLIER 180.0f / FB_PI

    // The multiplier to convert seconds to milliseconds.
#define FB_SEC_TO_MS_MULTIPLIER 1000.0f

// The multiplier to convert milliseconds to seconds.
#define FB_MS_TO_SEC_MULTIPLIER 0.001f

// A huge number that should be larger than any valid number used.
#define FB_INFINITY 1e30f

// Smallest positive number where 1.0 + FLOAT_EPSILON != 0
#define FB_FLOAT_EPSILON 1.192092896e-07f

#define FB_CLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max \
                                                                        : value;

    FBAPI f32 fbsin(f32 x);
    FBAPI f32 fbcos(f32 x);
    FBAPI f32 fbtan(f32 x);
    FBAPI f32 fbacos(f32 x);
    FBAPI f32 fbsqrt(f32 x);
    FBAPI f32 fbabs(f32 x);

    FBINLINE b8 is_power_2(u64 value) {
        return (value != 0) && ((value & (value - 1)) == 0);
    }

    namespace internal {
        i32 fbrandom(u32 seed, i32 min, i32 max);
        f32 fbrandom(u32 seed, f32 min, f32 max);
    }  // namespace internal

    template <typename T>
    FBAPI T fbrandom(u32 seed = -1, T min = 0, T max = 0) {
        return internal::fbrandom(seed, min, max);
    }

    FBAPI u64 max(u64 val1, u64 val2);
    FBAPI u64 min(u64 val1, u64 val2);

    FBINLINE f32 deg_to_rad(f32 degrees) {
        return degrees * FB_DEG2RAD_MULTIPLIER;
    }

    FBINLINE f32 rad_to_deg(f32 radians) {
        return radians * FB_RAD2DEG_MULTIPLIER;
    }

#include "ftl/vector.hpp"

    template <typename T>
    concept has_normalize = requires(T t) { t.normalize(); };

    template <typename T>
    concept has_length = requires(T t) { t.length(); };

    template <typename T>
        requires has_normalize<T>
    FBINLINE T normalize(const T& vec) {
        T result = vec;
        result.normalize();
        return result;
    }

    template <typename T>
        requires has_length<T>
    FBINLINE f32 distance(const T& vec0, const T& vec1) {
        T d = vec0 - vec1;
        return d.length();
    }

    /********** VEC2F **********/

    FBINLINE vec2f vec2_up() {
        return vec2f(0.0f, 1.0f);
    }

    FBINLINE vec2f vec2_down() {
        return vec2f(0.0f, -1.0f);
    }

    FBINLINE vec2f vec2_right() {
        return vec2f(1.0f, 0.0f);
    }

    FBINLINE vec2f vec2_left() {
        return vec2f(-1.0f, 0.0f);
    }

    FBINLINE f32 dot(const vec2f& vec0, const vec2f& vec1) {
        f32 result = 0.0f;
        result += vec0.vec[0] * vec1.vec[0];
        result += vec0.vec[1] * vec1.vec[1];

        return result;
    }

    /********** VEC3F **********/

    FBINLINE vec3f vec3_up() {
        return vec3f(0.0f, 0.0f, 1.0f);
    }

    FBINLINE vec3f vec3_down() {
        return vec3f(0.0f, 0.0f, -1.0f);
    }

    FBINLINE vec3f vec3_right() {
        return vec3f(1.0f, 0.0f, 0.0f);
    }

    FBINLINE vec3f vec3_left() {
        return vec3f(-1.0f, 0.0f, 0.0f);
    }

    FBINLINE vec3f vec3_forward() {
        return vec3f(0.0f, 1.0f, 0.0f);
    }

    FBINLINE vec3f vec3_backward() {
        return vec3f(0.0f, -1.0f, 0.0f);
    }

    FBINLINE f32 dot(const vec3f& vec0, const vec3f& vec1) {
        f32 result = 0.0f;
        result += vec0.vec[0] * vec1.vec[0];
        result += vec0.vec[1] * vec1.vec[1];
        result += vec0.vec[2] * vec1.vec[2];

        return result;
    }

    FBINLINE vec3f cross(const vec3f& vec0, const vec3f& vec1) {
        return vec3f(vec0.vec[1] * vec1.vec[2] - vec0.vec[2] * vec1.vec[1],
                     vec0.vec[2] * vec1.vec[0] - vec0.vec[0] * vec1.vec[2],
                     vec0.vec[0] * vec1.vec[1] - vec0.vec[1] * vec1.vec[0]);
    }

    /********** VEC4F **********/

    FBINLINE f32 dot(const vec4f& vec0, const vec4f& vec1) {
        f32 result = 0.0f;
        result += vec0.vec[0] * vec1.vec[0];
        result += vec0.vec[1] * vec1.vec[1];
        result += vec0.vec[2] * vec1.vec[2];
        result += vec0.vec[3] * vec1.vec[3];

        return result;
    }

#include "ftl/matrix.hpp"

    template <typename T>
    concept has_inverse = requires(T t) { t.inverse(); };

    template <typename T>
    concept has_transpose = requires(T t) { t.transpose(); };

    template <typename T>
        requires has_inverse<T>
    FBINLINE T inverse(const T& mat) {
        T result = mat;
        result.inverse();
        return result;
    }

    template <typename T>
        requires has_transpose<T>
    FBINLINE T transpose(const T& mat) {
        T result = mat;
        result.transpose();
        return result;
    }

    FBINLINE mat4 orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearClip, f32 farClip) {
        mat4 result = mat4(1.0f);

        f32 lr = 1.0f / (left - right);
        f32 bt = 1.0f / (bottom - top);
        f32 nf = 1.0f / (nearClip - farClip);

        result.mat[0] = -2.0f * lr;
        result.mat[5] = -2.0f * bt;
        result.mat[10] = 2.0f * nf;

        result.mat[12] = (left + right) * lr;
        result.mat[13] = (top + bottom) * bt;
        result.mat[14] = (farClip + nearClip) * nf;

        return result;
    }

    FBINLINE mat4 perspective(f32 fovRadians, f32 aspectRatio, f32 nearClip, f32 farClip) {
        f32 half_tan_fov = fbtan(fovRadians * 0.5f);

        mat4 result;

        result.mat[0] = 1.0f / (aspectRatio * half_tan_fov);
        result.mat[5] = 1.0f / half_tan_fov;
        result.mat[10] = -((farClip + nearClip) / (farClip - nearClip));
        result.mat[11] = -1.0f;
        result.mat[14] = -((2.0f * farClip * nearClip) / (farClip - nearClip));

        return result;
    }

    FBINLINE mat4 look_at(const vec3f& position, const vec3f& target, const vec3f& up) {
        mat4 result;
        vec3f y_axis;

        y_axis = target - position;
        y_axis.normalize();

        vec3f x_axis = normalize(cross(y_axis, up));
        vec3f z_axis = cross(x_axis, y_axis);

        result.row0 = vec4f(x_axis.x, z_axis.x, -y_axis.x, 0.0f);
        result.row1 = vec4f(x_axis.y, z_axis.y, -y_axis.y, 0.0f);
        result.row2 = vec4f(x_axis.z, z_axis.z, -y_axis.z, 0.0f);
        result.row3 = vec4f(-dot(x_axis, position), -dot(z_axis, position), 1.0f);

        return result;
    }

    FBINLINE mat4 translation(const vec3f& position) {
        mat4 result = mat4(1.0f);

        result.row3 = vec4f(position, 1.0f);

        return result;
    }

    FBINLINE mat4 scale(const vec3f& scale) {
        mat4 result = mat4(1.0f);

        result.mat[0] = scale.x;
        result.mat[5] = scale.y;
        result.mat[10] = scale.z;

        return result;
    }

    FBINLINE mat4 pitch(f32 angleRadians) {
        mat4 result = mat4(1.0f);

        f32 c = fbcos(angleRadians);
        f32 s = fbsin(angleRadians);

        result.mat[5] = c;
        result.mat[6] = s;
        result.mat[9] = -s;
        result.mat[10] = c;

        return result;
    }

    FBINLINE mat4 roll(f32 angleRadians) {
        mat4 result = mat4(1.0f);

        f32 c = fbcos(angleRadians);
        f32 s = fbsin(angleRadians);

        result.mat[0] = c;
        result.mat[1] = s;
        result.mat[4] = -s;
        result.mat[5] = c;

        return result;
    }

    FBINLINE mat4 yaw(f32 angleRadians) {
        mat4 result = mat4(1.0f);

        f32 c = fbcos(angleRadians);
        f32 s = fbsin(angleRadians);

        result.mat[0] = c;
        result.mat[2] = -s;
        result.mat[8] = s;
        result.mat[10] = c;

        return result;
    }

    FBINLINE mat4 rotation(f32 pitchRadians, f32 yawRadians, f32 rollRadians) {
        return pitch(pitchRadians) * yaw(yawRadians) * roll(rollRadians);
    }

    FBINLINE vec3f mat4_forward(const mat4& matrix) {
        vec3f result;
        result.x = -matrix.mat[2];
        result.y = -matrix.mat[6];
        result.z = -matrix.mat[10];
        result.normalize();

        return result;
    }

    FBINLINE vec3f mat4_backward(const mat4& matrix) {
        vec3f result;
        result.x = matrix.mat[2];
        result.y = matrix.mat[6];
        result.z = matrix.mat[10];
        result.normalize();

        return result;
    }

    FBINLINE vec3f mat4_up(const mat4& matrix) {
        vec3f result;
        result.x = matrix.mat[1];
        result.y = matrix.mat[5];
        result.z = matrix.mat[9];
        result.normalize();

        return result;
    }

    FBINLINE vec3f mat4_down(const mat4& matrix) {
        vec3f result;
        result.x = -matrix.mat[1];
        result.y = -matrix.mat[5];
        result.z = -matrix.mat[9];
        result.normalize();

        return result;
    }

    FBINLINE vec3f mat4_right(const mat4& matrix) {
        vec3f result;
        result.x = matrix.mat[0];
        result.y = matrix.mat[4];
        result.z = matrix.mat[8];
        result.normalize();

        return result;
    }

    FBINLINE vec3f mat4_left(const mat4& matrix) {
        vec3f result;
        result.x = -matrix.mat[0];
        result.y = -matrix.mat[4];
        result.z = -matrix.mat[8];
        result.normalize();

        return result;
    }

#include "ftl/quaternion.hpp"

    FBINLINE quat normalize(const quat& q) {
        quat result = q;
        result.normalize();
        return result;
    }

    FBINLINE quat inverse(const quat& q) {
        return normalize(q.conjugate());
    }

    FBINLINE f32 dot(const quat& q0, const quat& q1) {
        return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
    }

    FBINLINE mat4 rotation(const quat& q) {
        mat4 result = mat4(1.0f);
        quat n = normalize(q);

        result.mat[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
        result.mat[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
        result.mat[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

        result.mat[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
        result.mat[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
        result.mat[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

        result.mat[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
        result.mat[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
        result.mat[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

        return result;
    }

    FBINLINE mat4 rotation(const quat& q, const vec3f& center) {
        mat4 result;

        f32* o = result.mat;
        o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
        o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
        o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
        o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

        o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
        o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
        o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
        o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

        o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
        o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
        o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
        o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

        o[12] = 0.0f;
        o[13] = 0.0f;
        o[14] = 0.0f;
        o[15] = 1.0f;

        return result;
    }

    FBINLINE quat slerp(const quat& q0, const quat& q1, f32 percentage) {
        quat result;

        quat v0 = normalize(q0);
        quat v1 = normalize(q1);

        f32 d = dot(v0, v1);

        if (d < 0.0f) {
            v1.x = -v1.x;
            v1.y = -v1.y;
            v1.z = -v1.z;
            v1.w = -v1.w;
            d = -d;
        }

        const f32 DOT_THRESHOLD = 0.9995f;
        if (d > DOT_THRESHOLD) {
            result = {
                v0.x + ((v1.x - v0.x) * percentage),
                v0.y + ((v1.y - v0.y) * percentage),
                v0.z + ((v1.z - v0.z) * percentage),
                v0.w + ((v1.w - v0.w) * percentage)};

            return normalize(result);
        }

        f32 theta_0 = fbacos(d);
        f32 theta = theta_0 * percentage;
        f32 sin_theta = fbsin(theta);
        f32 sin_theta_0 = fbsin(theta_0);

        f32 s0 = fbcos(theta) - d * sin_theta / sin_theta_0;
        f32 s1 = sin_theta / sin_theta_0;

        return {
            (v0.x * s0) + (v1.x * s1),
            (v0.y * s0) + (v1.y * s1),
            (v0.z * s0) + (v1.z * s1),
            (v0.w * s0) + (v1.w * s1)};
    }
}  // namespace ftl