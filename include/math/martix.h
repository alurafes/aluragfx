#ifndef ALURA_MATH_MATRIX_H
#define ALURA_MATH_MATRIX_H

#include "vector.h"

typedef struct agfx_mat2x2_t {
    agfx_vector2_t mat[2];
} agfx_mat2x2_t;

typedef struct agfx_mat3x3_t {
    agfx_vector3_t mat[3];
} agfx_mat3x3_t;

typedef struct agfx_mat4x4_t {
    agfx_vector4_t mat[4];
} agfx_mat4x4_t;

agfx_mat2x2_t agfx_mat2x2_create_diagonal(float value);
agfx_mat3x3_t agfx_mat3x3_create_diagonal(float value);
agfx_mat4x4_t agfx_mat4x4_create_diagonal(float value);

agfx_vector2_t agfx_mat2x2_multiplied_by_vector2(agfx_mat2x2_t matrix, agfx_vector2_t vector);
agfx_vector3_t agfx_mat3x3_multiplied_by_vector3(agfx_mat3x3_t matrix, agfx_vector3_t vector);
agfx_vector4_t agfx_mat4x4_multiplied_by_vector4(agfx_mat4x4_t matrix, agfx_vector4_t vector);

agfx_mat2x2_t agfx_mat2x2_multiplied_by_mat2x2(agfx_mat2x2_t matrix_a, agfx_mat2x2_t matrix_b);
agfx_mat3x3_t agfx_mat3x3_multiplied_by_mat3x3(agfx_mat3x3_t matrix_a, agfx_mat3x3_t matrix_b);
agfx_mat4x4_t agfx_mat4x4_multiplied_by_mat4x4(agfx_mat4x4_t matrix_a, agfx_mat4x4_t matrix_b);

agfx_mat4x4_t agfx_mat4x4_translation(agfx_vector3_t position);
agfx_mat4x4_t agfx_mat4x4_rotation_euler(agfx_vector3_t rotation);
agfx_mat4x4_t agfx_mat4x4_scale(agfx_vector3_t scale);

agfx_mat4x4_t agfx_mat4x4_look_at(agfx_vector3_t camera, agfx_vector3_t subject, agfx_vector3_t up);
agfx_mat4x4_t agfx_mat4x4_perspective(float fov, float aspect_ratio, float z_near, float z_far);

#endif