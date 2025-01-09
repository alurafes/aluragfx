#include "math/vector.h"

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
