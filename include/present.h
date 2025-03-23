#ifndef AGFX_PRESENT_H
#define AGFX_PRESENT_H

#include "engine_types.h"

agfx_result_t agfx_create_present(agfx_present_t* out_present);
void agfx_free_present(agfx_present_t* present);

agfx_result_t create_window(agfx_present_t* present);

void free_window(agfx_present_t* present);

#endif