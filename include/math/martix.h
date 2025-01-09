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

#endif