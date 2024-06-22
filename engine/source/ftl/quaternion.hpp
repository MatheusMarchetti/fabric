#pragma once

struct quat {
    quat() : vec{1.0f, 0.0f, 0.0f, 0.0f} {}
    quat(f32 X, f32 Y, f32 Z, f32 W) : vec{W, X, Y, Z} {}
    quat(const vec3f& axis, f32 angle, b8 normalized = false) {
        const f32 half_angle = 0.5f * angle;
        f32 s = fbsin(half_angle);
        f32 c = fbcos(half_angle);

        vec[0] = c;
        vec[1] = s * axis.x;
        vec[2] = s * axis.y;
        vec[3] = s * axis.z;

        if(normalized) {
            normalize();
        }
    }

    f32 normal() { return fbsqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]); }

    quat operator*(const quat& other) {
        quat result;

        result.x = x * other.w +
                   y * other.z -
                   z * other.y +
                   w * other.x;

        result.y = -x * other.z +
                   y * other.w +
                   z * other.x +
                   w * other.y;

        result.z = x * other.y -
                   y * other.x +
                   z * other.w +
                   w * other.z;

        result.w = -x * other.x -
                   y * other.y -
                   z * other.z +
                   w * other.w;

        return result;
    }

    void normalize() {
        f32 n = normal();
        vec[0] /= n;
        vec[1] /= n;
        vec[2] /= n;
        vec[3] /= n;
    }

    quat conjugate() const {
        quat result;
        result.vec[0] = vec[0];
        result.vec[1] = -vec[1];
        result.vec[2] = -vec[2];
        result.vec[3] = -vec[3];

        return result;
    }

    union {
        f32 vec[4];
        struct {
            f32 w;
            f32 x;
            f32 y;
            f32 z;
        };
    };
};
