#ifndef ALURA_MATH_VECTOR_H
#define ALURA_MATH_VECTOR_H

typedef struct agfx_vector2_t {
    float x;
    float y;
} agfx_vector2_t;

typedef struct agfx_vector3_t {
    float x;
    float y;
    float z;
} agfx_vector3_t;

typedef struct agfx_vector4_t {
    float x;
    float y;
    float z;
    float w;
} agfx_vector4_t;

agfx_vector3_t agfx_vector3_from_vector2(agfx_vector2_t vector, float z);
agfx_vector4_t agfx_vector4_from_vector2(agfx_vector2_t vector, float z, float w);
agfx_vector4_t agfx_vector4_from_vector3(agfx_vector3_t vector, float w);

#endif