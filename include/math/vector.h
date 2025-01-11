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

agfx_vector3_t agfx_vector3_cross(agfx_vector3_t a, agfx_vector3_t b);

float agfx_vector2_dot(agfx_vector2_t a, agfx_vector2_t b);
float agfx_vector3_dot(agfx_vector3_t a, agfx_vector3_t b);
float agfx_vector4_dot(agfx_vector4_t a, agfx_vector4_t b);

float agfx_vector2_magnitude(agfx_vector2_t vector);
float agfx_vector3_magnitude(agfx_vector3_t vector);
float agfx_vector4_magnitude(agfx_vector4_t vector);

agfx_vector2_t agfx_vector2_normalize(agfx_vector2_t vector);
agfx_vector3_t agfx_vector3_normalize(agfx_vector3_t vector);
agfx_vector4_t agfx_vector4_normalize(agfx_vector4_t vector);

agfx_vector2_t agfx_vector2_add_vector2(agfx_vector2_t a, agfx_vector2_t b);
agfx_vector3_t agfx_vector3_add_vector3(agfx_vector3_t a, agfx_vector3_t b);
agfx_vector4_t agfx_vector4_add_vector4(agfx_vector4_t a, agfx_vector4_t b);

agfx_vector2_t agfx_vector2_subtract_vector2(agfx_vector2_t a, agfx_vector2_t b);
agfx_vector3_t agfx_vector3_subtract_vector3(agfx_vector3_t a, agfx_vector3_t b);
agfx_vector4_t agfx_vector4_subtract_vector4(agfx_vector4_t a, agfx_vector4_t b);

agfx_vector2_t agfx_vector2_multiply_vector2(agfx_vector2_t a, agfx_vector2_t b);
agfx_vector3_t agfx_vector3_multiply_vector3(agfx_vector3_t a, agfx_vector3_t b);
agfx_vector4_t agfx_vector4_multiply_vector4(agfx_vector4_t a, agfx_vector4_t b);

agfx_vector2_t agfx_vector2_divide_vector2(agfx_vector2_t a, agfx_vector2_t b);
agfx_vector3_t agfx_vector3_divide_vector3(agfx_vector3_t a, agfx_vector3_t b);
agfx_vector4_t agfx_vector4_divide_vector4(agfx_vector4_t a, agfx_vector4_t b);

agfx_vector2_t agfx_vector2_add_scalar(agfx_vector2_t a, float b);
agfx_vector3_t agfx_vector3_add_scalar(agfx_vector3_t a, float b);
agfx_vector4_t agfx_vector4_add_scalar(agfx_vector4_t a, float b);

agfx_vector2_t agfx_vector2_subtract_scalar(agfx_vector2_t a, float b);
agfx_vector3_t agfx_vector3_subtract_scalar(agfx_vector3_t a, float b);
agfx_vector4_t agfx_vector4_subtract_scalar(agfx_vector4_t a, float b);

agfx_vector2_t agfx_vector2_multiply_scalar(agfx_vector2_t a, float b);
agfx_vector3_t agfx_vector3_multiply_scalar(agfx_vector3_t a, float b);
agfx_vector4_t agfx_vector4_multiply_scalar(agfx_vector4_t a, float b);

agfx_vector2_t agfx_vector2_divide_scalar(agfx_vector2_t a, float b);
agfx_vector3_t agfx_vector3_divide_scalar(agfx_vector3_t a, float b);
agfx_vector4_t agfx_vector4_divide_scalar(agfx_vector4_t a, float b);

#endif