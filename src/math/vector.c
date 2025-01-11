#include "math/vector.h"

#include <math.h>

agfx_vector3_t agfx_vector3_from_vector2(agfx_vector2_t vector, float z)
{
    return (agfx_vector3_t) {
        .x = vector.x,
        .y = vector.y,
        .z = z
    };
}

agfx_vector4_t agfx_vector4_from_vector2(agfx_vector2_t vector, float z, float w)
{
    return (agfx_vector4_t) {
        .x = vector.x,
        .y = vector.y,
        .z = z,
        .w = w
    };
}

agfx_vector4_t agfx_vector4_from_vector3(agfx_vector3_t vector, float w)
{
    return (agfx_vector4_t) {
        .x = vector.x,
        .y = vector.y,
        .z = vector.z,
        .w = w
    };
}

agfx_vector3_t agfx_vector3_cross(agfx_vector3_t a, agfx_vector3_t b) 
{
    return (agfx_vector3_t) {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
}


float agfx_vector2_dot(agfx_vector2_t a, agfx_vector2_t b)
{
    return a.x * b.x + a.y * b.y;
}

float agfx_vector3_dot(agfx_vector3_t a, agfx_vector3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float agfx_vector4_dot(agfx_vector4_t a, agfx_vector4_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float agfx_vector2_magnitude(agfx_vector2_t vector)
{
    return sqrtf(vector.x * vector.x + vector.y * vector.y);
}

float agfx_vector3_magnitude(agfx_vector3_t vector)
{
    return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

float agfx_vector4_magnitude(agfx_vector4_t vector)
{
    return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w);
}

agfx_vector2_t agfx_vector2_normalize(agfx_vector2_t vector)
{
    float magnitude = agfx_vector2_magnitude(vector);
    return (agfx_vector2_t) {
        .x = vector.x / magnitude,
        .y = vector.y / magnitude,  
    };
}

agfx_vector3_t agfx_vector3_normalize(agfx_vector3_t vector)
{
    float magnitude = agfx_vector3_magnitude(vector);
    if (magnitude == 0.0f) {
        return (agfx_vector3_t) {0.0f, 0.0f, 0.0f}; // Return zero vector if input is zero vector
    }
    return (agfx_vector3_t) {
        .x = vector.x / magnitude,
        .y = vector.y / magnitude,
        .z = vector.z / magnitude
    };
}

agfx_vector4_t agfx_vector4_normalize(agfx_vector4_t vector)
{
    float magnitude = agfx_vector4_magnitude(vector);
    return (agfx_vector4_t) {
        .x = vector.x / magnitude,
        .y = vector.y / magnitude,
        .z = vector.z / magnitude,
        .w = vector.w / magnitude
    };
}

agfx_vector2_t agfx_vector2_add_vector2(agfx_vector2_t a, agfx_vector2_t b)
{
    return (agfx_vector2_t) {
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

agfx_vector3_t agfx_vector3_add_vector3(agfx_vector3_t a, agfx_vector3_t b)
{
    return (agfx_vector3_t) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

agfx_vector4_t agfx_vector4_add_vector4(agfx_vector4_t a, agfx_vector4_t b)
{
    return (agfx_vector4_t) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
        .w = a.w + b.w
    };
}

agfx_vector2_t agfx_vector2_subtract_vector2(agfx_vector2_t a, agfx_vector2_t b)
{
    return (agfx_vector2_t) {
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
}

agfx_vector3_t agfx_vector3_subtract_vector3(agfx_vector3_t a, agfx_vector3_t b)
{
    return (agfx_vector3_t) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

agfx_vector4_t agfx_vector4_subtract_vector4(agfx_vector4_t a, agfx_vector4_t b)
{
    return (agfx_vector4_t) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
        .w = a.w - b.w
    };
}

agfx_vector2_t agfx_vector2_multiply_vector2(agfx_vector2_t a, agfx_vector2_t b)
{
    return (agfx_vector2_t) {
        .x = a.x * b.x,
        .y = a.y * b.y,
    };
}

agfx_vector3_t agfx_vector3_multiply_vector3(agfx_vector3_t a, agfx_vector3_t b)
{
    return (agfx_vector3_t) {
        .x = a.x * b.x,
        .y = a.y * b.y,
        .z = a.z * b.z
    };
}

agfx_vector4_t agfx_vector4_multiply_vector4(agfx_vector4_t a, agfx_vector4_t b)
{
    return (agfx_vector4_t) {
        .x = a.x * b.x,
        .y = a.y * b.y,
        .z = a.z * b.z,
        .w = a.w * b.w
    };
}

agfx_vector2_t agfx_vector2_divide_vector2(agfx_vector2_t a, agfx_vector2_t b)
{
    return (agfx_vector2_t) {
        .x = a.x / b.x,
        .y = a.y / b.y,
    };
}

agfx_vector3_t agfx_vector3_divide_vector3(agfx_vector3_t a, agfx_vector3_t b)
{
    return (agfx_vector3_t) {
        .x = a.x / b.x,
        .y = a.y / b.y,
        .z = a.z / b.z
    };
}

agfx_vector4_t agfx_vector4_divide_vector4(agfx_vector4_t a, agfx_vector4_t b)
{
    return (agfx_vector4_t) {
        .x = a.x / b.x,
        .y = a.y / b.y,
        .z = a.z / b.z,
        .w = a.w / b.w
    };
}

agfx_vector2_t agfx_vector2_add_scalar(agfx_vector2_t a, float b)
{
    return (agfx_vector2_t) {
        .x = a.x + b,
        .y = a.y + b,
    };
}

agfx_vector3_t agfx_vector3_add_scalar(agfx_vector3_t a, float b)
{
    return (agfx_vector3_t) {
        .x = a.x + b,
        .y = a.y + b,
        .z = a.z + b,
    };
}

agfx_vector4_t agfx_vector4_add_scalar(agfx_vector4_t a, float b)
{
    return (agfx_vector4_t) {
        .x = a.x + b,
        .y = a.y + b,
        .z = a.z + b,
        .w = a.w + b,
    };
}

agfx_vector2_t agfx_vector2_subtract_scalar(agfx_vector2_t a, float b)
{
    return (agfx_vector2_t) {
        .x = a.x - b,
        .y = a.y - b,
    };
}

agfx_vector3_t agfx_vector3_subtract_scalar(agfx_vector3_t a, float b)
{
    return (agfx_vector3_t) {
        .x = a.x - b,
        .y = a.y - b,
        .z = a.z - b,
    };
}
agfx_vector4_t agfx_vector4_subtract_scalar(agfx_vector4_t a, float b)
{
    return (agfx_vector4_t) {
        .x = a.x - b,
        .y = a.y - b,
        .z = a.z - b,
        .w = a.w - b,
    };
}

agfx_vector2_t agfx_vector2_multiply_scalar(agfx_vector2_t a, float b)
{
    return (agfx_vector2_t) {
        .x = a.x * b,
        .y = a.y * b,
    };
}

agfx_vector3_t agfx_vector3_multiply_scalar(agfx_vector3_t a, float b)
{
    return (agfx_vector3_t) {
        .x = a.x * b,
        .y = a.y * b,
        .z = a.z * b,
    };
}

agfx_vector4_t agfx_vector4_multiply_scalar(agfx_vector4_t a, float b)
{
    return (agfx_vector4_t) {
        .x = a.x * b,
        .y = a.y * b,
        .z = a.z * b,
        .w = a.w * b,
    };
}

agfx_vector2_t agfx_vector2_divide_scalar(agfx_vector2_t a, float b)
{
    return (agfx_vector2_t) {
        .x = a.x / b,
        .y = a.y / b,
    };
}

agfx_vector3_t agfx_vector3_divide_scalar(agfx_vector3_t a, float b)
{
    return (agfx_vector3_t) {
        .x = a.x / b,
        .y = a.y / b,
        .z = a.z / b,
    };
}

agfx_vector4_t agfx_vector4_divide_scalar(agfx_vector4_t a, float b)
{
    return (agfx_vector4_t) {
        .x = a.x / b,
        .y = a.y / b,
        .z = a.z / b,
        .w = a.w / b,
    };
}
