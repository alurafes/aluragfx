#include "utils.h"

int unsigned_integer_compare(const void *a, const void *b)
{
    int value_a = *(uint32_t *)a;
    int value_b = *(uint32_t *)b;
    return value_a - value_b;
}