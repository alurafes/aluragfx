#ifndef AGFX_CONTEXT_H
#define AGFX_CONTEXT_H

#include "engine_types.h"
#include "utils.h"
#include <SDL2/SDL_vulkan.h>

agfx_result_t agfx_create_context(agfx_present_t* present, agfx_context_t* out_context);
void agfx_free_context(agfx_context_t* context);

agfx_result_t create_vulkan_instance(agfx_context_t* context);
agfx_result_t create_vulkan_surface(agfx_context_t* context);
agfx_result_t find_physical_device(agfx_context_t* context);
size_t get_unique_queue_family_indices(agfx_queue_family_indices_t* const queue_family_indices, uint32_t* output_indices_array);
agfx_result_t create_logical_device(agfx_context_t* context);

void free_vulkan_instance(agfx_context_t* context);
void free_logical_device(agfx_context_t* context);
void free_vulkan_surface(agfx_context_t* context);

#endif