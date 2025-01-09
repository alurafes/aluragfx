#include "math/martix.h"

#include <math.h>

agfx_mat2x2_t agfx_mat2x2_create_diagonal(float value)
{
    return (agfx_mat2x2_t) {
        .mat = {
            {value, 0.0f},
            {0.0f, value},
        }
    };
}

agfx_mat3x3_t agfx_mat3x3_create_diagonal(float value)
{
    return (agfx_mat3x3_t) {
        .mat = {
            {value, 0.0f, 0.0f},
            {0.0f, value, 0.0f},
            {0.0f, 0.0f, value},
        }
    };
}

agfx_mat4x4_t agfx_mat4x4_create_diagonal(float value)
{
    return (agfx_mat4x4_t) {
        .mat = {
            {value, 0.0f, 0.0f, 0.0f},
            {0.0f, value, 0.0f, 0.0f},
            {0.0f, 0.0f, value, 0.0f},
            {0.0f, 0.0f, 0.0f, value},
        }
    };
}

agfx_vector2_t agfx_mat2x2_multiplied_by_vector2(agfx_mat2x2_t matrix, agfx_vector2_t vector)
{
    return (agfx_vector2_t) {
        .x = matrix.mat[0].x * vector.x + matrix.mat[1].x * vector.y,
        .y = matrix.mat[0].y * vector.x + matrix.mat[1].y * vector.y,
    };
}

agfx_vector3_t agfx_mat3x3_multiplied_by_vector3(agfx_mat3x3_t matrix, agfx_vector3_t vector)
{
    return (agfx_vector3_t) {
        .x = matrix.mat[0].x * vector.x + matrix.mat[1].x * vector.y + matrix.mat[2].x * vector.z,
        .y = matrix.mat[0].y * vector.x + matrix.mat[1].y * vector.y + matrix.mat[2].y * vector.z,
        .z = matrix.mat[0].z * vector.x + matrix.mat[1].z * vector.y + matrix.mat[2].z * vector.z,
    };
}

agfx_vector4_t agfx_mat4x4_multiplied_by_vector4(agfx_mat4x4_t matrix, agfx_vector4_t vector)
{
    return (agfx_vector4_t) {
        .x = matrix.mat[0].x * vector.x + matrix.mat[1].x * vector.y + matrix.mat[2].x * vector.z + matrix.mat[3].x * vector.w,
        .y = matrix.mat[0].y * vector.x + matrix.mat[1].y * vector.y + matrix.mat[2].y * vector.z + matrix.mat[3].y * vector.w,
        .z = matrix.mat[0].z * vector.x + matrix.mat[1].z * vector.y + matrix.mat[2].z * vector.z + matrix.mat[3].z * vector.w,
        .w = matrix.mat[0].w * vector.x + matrix.mat[1].w * vector.y + matrix.mat[2].w * vector.z + matrix.mat[3].w * vector.w,
    };
}

agfx_mat2x2_t agfx_mat2x2_multiplied_by_mat2x2(agfx_mat2x2_t matrix_a, agfx_mat2x2_t matrix_b)
{
    return (agfx_mat2x2_t) {
        .mat = {
            {matrix_a.mat[0].x * matrix_b.mat[0].x + matrix_a.mat[1].x * matrix_b.mat[0].y, matrix_a.mat[0].x * matrix_b.mat[1].x + matrix_a.mat[1].x * matrix_b.mat[1].y},
            {matrix_a.mat[0].y * matrix_b.mat[0].x + matrix_a.mat[1].y * matrix_b.mat[0].y, matrix_a.mat[0].y * matrix_b.mat[1].x + matrix_a.mat[1].y * matrix_b.mat[1].y},
        }
    };
}

agfx_mat3x3_t agfx_mat3x3_multiplied_by_mat3x3(agfx_mat3x3_t matrix_a, agfx_mat3x3_t matrix_b)
{
    return (agfx_mat3x3_t) {
        .mat = {
            {matrix_a.mat[0].x * matrix_b.mat[0].x + matrix_a.mat[1].x * matrix_b.mat[0].y + matrix_a.mat[2].x * matrix_b.mat[0].z, matrix_a.mat[0].x * matrix_b.mat[1].x + matrix_a.mat[1].x * matrix_b.mat[1].y + matrix_a.mat[2].x * matrix_b.mat[1].z, matrix_a.mat[0].x * matrix_b.mat[2].x + matrix_a.mat[1].x * matrix_b.mat[2].y + matrix_a.mat[2].x * matrix_b.mat[2].z},
            {matrix_a.mat[0].y * matrix_b.mat[0].x + matrix_a.mat[1].y * matrix_b.mat[0].y + matrix_a.mat[2].y * matrix_b.mat[0].z, matrix_a.mat[0].y * matrix_b.mat[1].x + matrix_a.mat[1].y * matrix_b.mat[1].y + matrix_a.mat[2].y * matrix_b.mat[1].z, matrix_a.mat[0].y * matrix_b.mat[2].x + matrix_a.mat[1].y * matrix_b.mat[2].y + matrix_a.mat[2].y * matrix_b.mat[2].z},
            {matrix_a.mat[0].z * matrix_b.mat[0].x + matrix_a.mat[1].z * matrix_b.mat[0].y + matrix_a.mat[2].z * matrix_b.mat[0].z, matrix_a.mat[0].z * matrix_b.mat[1].x + matrix_a.mat[1].z * matrix_b.mat[1].y + matrix_a.mat[2].z * matrix_b.mat[1].z, matrix_a.mat[0].z * matrix_b.mat[2].x + matrix_a.mat[1].z * matrix_b.mat[2].y + matrix_a.mat[2].z * matrix_b.mat[2].z}
        }
    };
}

agfx_mat4x4_t agfx_mat4x4_multiplied_by_mat4x4(agfx_mat4x4_t matrix_a, agfx_mat4x4_t matrix_b)
{
    return (agfx_mat4x4_t) {
        .mat = {
            {
                matrix_a.mat[0].x * matrix_b.mat[0].x + matrix_a.mat[1].x * matrix_b.mat[0].y + matrix_a.mat[2].x * matrix_b.mat[0].z + matrix_a.mat[3].x * matrix_b.mat[0].w,
                matrix_a.mat[0].x * matrix_b.mat[1].x + matrix_a.mat[1].x * matrix_b.mat[1].y + matrix_a.mat[2].x * matrix_b.mat[1].z + matrix_a.mat[3].x * matrix_b.mat[1].w,
                matrix_a.mat[0].x * matrix_b.mat[2].x + matrix_a.mat[1].x * matrix_b.mat[2].y + matrix_a.mat[2].x * matrix_b.mat[2].z + matrix_a.mat[3].x * matrix_b.mat[2].w,
                matrix_a.mat[0].x * matrix_b.mat[3].x + matrix_a.mat[1].x * matrix_b.mat[3].y + matrix_a.mat[2].x * matrix_b.mat[3].z + matrix_a.mat[3].x * matrix_b.mat[3].w
            },
            {
                matrix_a.mat[0].y * matrix_b.mat[0].x + matrix_a.mat[1].y * matrix_b.mat[0].y + matrix_a.mat[2].y * matrix_b.mat[0].z + matrix_a.mat[3].y * matrix_b.mat[0].w,
                matrix_a.mat[0].y * matrix_b.mat[1].x + matrix_a.mat[1].y * matrix_b.mat[1].y + matrix_a.mat[2].y * matrix_b.mat[1].z + matrix_a.mat[3].y * matrix_b.mat[1].w,
                matrix_a.mat[0].y * matrix_b.mat[2].x + matrix_a.mat[1].y * matrix_b.mat[2].y + matrix_a.mat[2].y * matrix_b.mat[2].z + matrix_a.mat[3].y * matrix_b.mat[2].w,
                matrix_a.mat[0].y * matrix_b.mat[3].x + matrix_a.mat[1].y * matrix_b.mat[3].y + matrix_a.mat[2].y * matrix_b.mat[3].z + matrix_a.mat[3].y * matrix_b.mat[3].w
            },
            {
                matrix_a.mat[0].z * matrix_b.mat[0].x + matrix_a.mat[1].z * matrix_b.mat[0].y + matrix_a.mat[2].z * matrix_b.mat[0].z + matrix_a.mat[3].z * matrix_b.mat[0].w,
                matrix_a.mat[0].z * matrix_b.mat[1].x + matrix_a.mat[1].z * matrix_b.mat[1].y + matrix_a.mat[2].z * matrix_b.mat[1].z + matrix_a.mat[3].z * matrix_b.mat[1].w,
                matrix_a.mat[0].z * matrix_b.mat[2].x + matrix_a.mat[1].z * matrix_b.mat[2].y + matrix_a.mat[2].z * matrix_b.mat[2].z + matrix_a.mat[3].z * matrix_b.mat[2].w,
                matrix_a.mat[0].z * matrix_b.mat[3].x + matrix_a.mat[1].z * matrix_b.mat[3].y + matrix_a.mat[2].z * matrix_b.mat[3].z + matrix_a.mat[3].z * matrix_b.mat[3].w
            },
            {
                matrix_a.mat[0].w * matrix_b.mat[0].x + matrix_a.mat[1].w * matrix_b.mat[0].y + matrix_a.mat[2].w * matrix_b.mat[0].z + matrix_a.mat[3].w * matrix_b.mat[0].w,
                matrix_a.mat[0].w * matrix_b.mat[1].x + matrix_a.mat[1].w * matrix_b.mat[1].y + matrix_a.mat[2].w * matrix_b.mat[1].z + matrix_a.mat[3].w * matrix_b.mat[1].w,
                matrix_a.mat[0].w * matrix_b.mat[2].x + matrix_a.mat[1].w * matrix_b.mat[2].y + matrix_a.mat[2].w * matrix_b.mat[2].z + matrix_a.mat[3].w * matrix_b.mat[2].w,
                matrix_a.mat[0].w * matrix_b.mat[3].x + matrix_a.mat[1].w * matrix_b.mat[3].y + matrix_a.mat[2].w * matrix_b.mat[3].z + matrix_a.mat[3].w * matrix_b.mat[3].w
            },
        }
    };
}

agfx_mat4x4_t agfx_mat4x4_translation(agfx_vector3_t position)
{
    agfx_mat4x4_t translation_matrix = agfx_mat4x4_create_diagonal(1.0f);

    translation_matrix.mat[3].x = position.x;
    translation_matrix.mat[3].y = position.y;
    translation_matrix.mat[3].z = position.z;

    return translation_matrix;
}

agfx_mat4x4_t agfx_mat4x4_rotation_euler(agfx_vector3_t rotation)
{
    float cos_of_x = cosf(rotation.x);
    float sin_of_x = sinf(rotation.x);
    float cos_of_y = cosf(rotation.y);
    float sin_of_y = sinf(rotation.y);
    float cos_of_z = cosf(rotation.z);
    float sin_of_z = sinf(rotation.z);
    float cos_of_x_times_sin_of_y = cos_of_x * sin_of_y;
    float sin_of_x_times_sin_of_y = sin_of_x * sin_of_y;

    agfx_mat4x4_t rotation_matrix = agfx_mat4x4_create_diagonal(0.0f);
    rotation_matrix.mat[0].x = cos_of_y * cos_of_z;
    rotation_matrix.mat[1].x = -cos_of_y * sin_of_z;
    rotation_matrix.mat[2].x = sin_of_y;
    rotation_matrix.mat[0].y = sin_of_x_times_sin_of_y * cos_of_z + cos_of_x * sin_of_z;
    rotation_matrix.mat[1].y = -sin_of_x_times_sin_of_y * sin_of_z + cos_of_x * cos_of_z;
    rotation_matrix.mat[2].y = -sin_of_x * cos_of_y;
    rotation_matrix.mat[0].z = -cos_of_x_times_sin_of_y * cos_of_z + sin_of_x * sin_of_z;
    rotation_matrix.mat[1].z = cos_of_x_times_sin_of_y * sin_of_z + sin_of_x * cos_of_z;
    rotation_matrix.mat[2].z = cos_of_x * cos_of_y;
    rotation_matrix.mat[3].w = 1.0f;

    return rotation_matrix;
}

agfx_mat4x4_t agfx_mat4x4_scale(agfx_vector3_t scale)
{
    agfx_mat4x4_t scale_matrix = agfx_mat4x4_create_diagonal(1.0f);

    scale_matrix.mat[0].x = scale.x;
    scale_matrix.mat[1].y = scale.y;
    scale_matrix.mat[2].z = scale.z;

    return scale_matrix;
}