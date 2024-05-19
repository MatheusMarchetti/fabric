#pragma once

#ifdef __cplusplus
struct mat3 {
    mat3(f32 diag = 0.0f) : mat{diag, 0.0f, 0.0f,
                                0.0f, diag, 0.0f,
                                0.0f, 0.0f, diag} {}

    mat3(vec3f r0, vec3f r1, vec3f r2) : mat{r0.x, r0.y, r0.z,
                                             r1.x, r1.y, r1.z,
                                             r2.x, r2.y, r2.z} {}

    mat3 operator*(f32 scalar) {
        return mat3(row0 * scalar, row1 * scalar, row2 * scalar);
    }

    mat3 operator*(mat3& other) {
        mat3 result = mat3(1.0f);

        const f32* m1_ptr = mat;
        const f32* m2_ptr = other.mat;
        f32* result_ptr = result.mat;

        for (u32 i = 0; i < 3; i++) {
            for (u32 j = 0; j < 3; j++) {
                *result_ptr = m1_ptr[0] * m2_ptr[0 + j] +
                              m1_ptr[1] * m2_ptr[3 + j] +
                              m1_ptr[2] * m2_ptr[6 + j];
                result_ptr++;
            }
            m1_ptr += 3;
        }

        return result;
    }

    mat3 transpose() {
        mat3 result = mat3(1.0f);

        result.mat[0] = mat[0];
        result.mat[1] = mat[3];
        result.mat[2] = mat[6];
        result.mat[3] = mat[1];
        result.mat[4] = mat[4];
        result.mat[5] = mat[7];
        result.mat[6] = mat[2];
        result.mat[7] = mat[5];
        result.mat[8] = mat[8];

        return result;
    }

    void inverse() {
        mat3 result;

        // TODO:

        f32* m = mat;
        const f32* r = result.mat;

        *m = *r;
    }

    union {
        f32 mat[9];
        vec3f row0;
        vec3f row1;
        vec3f row2;
    };
};

struct mat4 {
    mat4(f32 diag = 0.0f) : mat{diag, 0.0f, 0.0f, 0.0f,
                                0.0f, diag, 0.0f, 0.0f,
                                0.0f, 0.0f, diag, 0.0f,
                                0.0f, 0.0f, 0.0f, diag} {}

    mat4(const vec4f& r0, const vec4f& r1, const vec4f& r2, const vec4f& r3) : mat{r0.x, r0.y, r0.z, r0.w,
                                                                                   r1.x, r1.y, r1.z, r1.w,
                                                                                   r2.x, r2.y, r2.z, r2.w,
                                                                                   r3.x, r3.y, r3.z, r3.w} {}

    mat4 operator*(f32 scalar) {
        return mat4(row0 * scalar, row1 * scalar, row2 * scalar, row3 * scalar);
    }

    mat4 operator*(const mat4& other) {
        mat4 result = mat4(1.0f);

        const f32* m1_ptr = mat;
        const f32* m2_ptr = other.mat;
        f32* result_ptr = result.mat;

        for (u32 i = 0; i < 4; i++) {
            for (u32 j = 0; j < 4; j++) {
                *result_ptr = m1_ptr[0] * m2_ptr[0 + j] +
                              m1_ptr[1] * m2_ptr[4 + j] +
                              m1_ptr[2] * m2_ptr[8 + j] +
                              m1_ptr[3] * m2_ptr[12 + j];
                result_ptr++;
            }
            m1_ptr += 4;
        }

        return result;
    }

    void transpose() {
        mat4 result = mat4(1.0f);

        result.mat[0] = mat[0];
        result.mat[1] = mat[4];
        result.mat[2] = mat[8];
        result.mat[3] = mat[12];
        result.mat[4] = mat[1];
        result.mat[5] = mat[5];
        result.mat[6] = mat[9];
        result.mat[7] = mat[13];
        result.mat[8] = mat[2];
        result.mat[9] = mat[6];
        result.mat[10] = mat[10];
        result.mat[11] = mat[14];
        result.mat[12] = mat[3];
        result.mat[13] = mat[7];
        result.mat[14] = mat[11];
        result.mat[15] = mat[15];

        f32* m = mat;
        f32* r = result.mat;

        *m = *r;
    }

    void inverse() {
        f32* m = mat;

        f32 t0 = m[10] * m[15];
        f32 t1 = m[14] * m[11];
        f32 t2 = m[6] * m[15];
        f32 t3 = m[14] * m[7];
        f32 t4 = m[6] * m[11];
        f32 t5 = m[10] * m[7];
        f32 t6 = m[2] * m[15];
        f32 t7 = m[14] * m[3];
        f32 t8 = m[2] * m[11];
        f32 t9 = m[10] * m[3];
        f32 t10 = m[2] * m[7];
        f32 t11 = m[6] * m[3];
        f32 t12 = m[8] * m[13];
        f32 t13 = m[12] * m[9];
        f32 t14 = m[4] * m[13];
        f32 t15 = m[12] * m[5];
        f32 t16 = m[4] * m[9];
        f32 t17 = m[8] * m[5];
        f32 t18 = m[0] * m[13];
        f32 t19 = m[12] * m[1];
        f32 t20 = m[0] * m[9];
        f32 t21 = m[8] * m[1];
        f32 t22 = m[0] * m[5];
        f32 t23 = m[4] * m[1];

        mat4 result;
        f32* o = result.mat;

        o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
        o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
        o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
        o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

        f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

        o[0] = d * o[0];
        o[1] = d * o[1];
        o[2] = d * o[2];
        o[3] = d * o[3];
        o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
        o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
        o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
        o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
        o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
        o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
        o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
        o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
        o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
        o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
        o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
        o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

        *m = *o;
    }

    union {
        f32 mat[16];
        vec4f row0;
        vec4f row1;
        vec4f row2;
        vec4f row3;
    };
};
#else
using mat3 = float3x3;
using mat4 = float4x4;
#endif