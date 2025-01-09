#include "math/martix.h"

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