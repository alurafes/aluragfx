#ifndef AGFX_SWAPCHAIN_T
#define AGFX_SWAPCHAIN_T

#include "engine_types.h"
#include "helper.h"

#include <SDL2/SDL_vulkan.h>
#include <math.h>

static agfx_result_t populate_swapchain_info(agfx_swapchain_t *swapchain);
static agfx_result_t create_swapchain(agfx_swapchain_t *swapchain);
static agfx_result_t create_swapchain_image_views(agfx_swapchain_t *swapchain);
static agfx_result_t create_framebuffers(agfx_swapchain_t *swapchain);
static agfx_result_t create_depth_resources(agfx_swapchain_t *swapchain);

static void free_swapchain_info(agfx_swapchain_t* swapchain);
static void free_swapchain(agfx_swapchain_t* swapchain);
static void free_swapchain_image_views(agfx_swapchain_t* swapchain);
static void free_framebuffers(agfx_swapchain_t* swapchain);
static void free_depth_resources(agfx_swapchain_t* swapchain);

agfx_result_t agfx_create_swapchain(agfx_context_t* context, agfx_present_t* present, agfx_swapchain_t* out_swapchain);
agfx_result_t agfx_recreate_swapchain(agfx_swapchain_t *swapchain);
void agfx_free_swapchain(agfx_swapchain_t* swapchain);
agfx_result_t agfx_create_framebuffers(agfx_swapchain_t *swapchain);


#endif