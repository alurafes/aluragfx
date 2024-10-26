#ifndef AGFX_ENGINE_H
#define AGFX_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "engine_types.h"
#include "present.h"
#include "context.h"
#include "swapchain.h"
#include "renderer.h"

agfx_result_t agfx_initialize_engine(agfx_engine_t* out_engine);
void agfx_game_loop(agfx_engine_t* engine);
void agfx_main(agfx_engine_t* engine);
void agfx_free_engine(agfx_engine_t* engine);

#endif