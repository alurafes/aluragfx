#ifndef AGFX_RENDERER_H
#define AGFX_RENDERER_H

#include "engine_types.h"
#include "helper.h"

#include <sys/stat.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <SDL2/SDL_image.h>

#define AGFX_MAX_FRAMES_IN_FLIGHT 2
#define AGFX_DESCRIPTOR_COUNT 2

#define AGFX_VERTEX_ARRAY_SIZE 24
static const agfx_vertex_t agfx_vertices[AGFX_VERTEX_ARRAY_SIZE] = {
     {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, 
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, 
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.5f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.5f, 0.5f, 1.0f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.5f, 1.0f, 0.5f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.5f, 0.5f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.5f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.5f, 1.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.5f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.5f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.5f, 0.0f, 0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.5f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.5f, 1.0f, 0.5f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.5f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.5f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.5f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.5f}, {0.0f, 1.0f}},
};

#define AGFX_INDEX_ARRAY_SIZE 36
static const uint32_t agfx_indices[AGFX_INDEX_ARRAY_SIZE] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

static const VkVertexInputBindingDescription agfx_vertex_input_binding_description = {
    .binding = 0,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    .stride = sizeof(agfx_vertex_t)
};

#define AGFX_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_SIZE 3
static const VkVertexInputAttributeDescription agfx_vertex_input_attribute_description[AGFX_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_SIZE] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(agfx_vertex_t, position)
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(agfx_vertex_t, color)
    },
    {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(agfx_vertex_t, texture_coordinate)
    }
};

static agfx_result_t create_descriptor_set_layout(agfx_renderer_t *renderer);
static agfx_result_t create_descriptor_pool(agfx_renderer_t *renderer);
static agfx_result_t create_descriptor_sets(agfx_renderer_t *renderer);
static agfx_result_t create_pipeline(agfx_renderer_t *renderer);
static agfx_result_t create_render_pass(agfx_renderer_t *renderer);
static agfx_result_t create_command_pool(agfx_renderer_t *renderer);
static agfx_result_t create_command_buffers(agfx_renderer_t *renderer);
static agfx_result_t create_sync_objects(agfx_renderer_t *renderer);
static agfx_result_t create_vertex_buffer(agfx_renderer_t *renderer);
static agfx_result_t create_index_buffer(agfx_renderer_t *renderer);
static agfx_result_t create_uniform_buffers(agfx_renderer_t *renderer);
static agfx_result_t create_texture_image(agfx_renderer_t *renderer);
static agfx_result_t create_texture_image_view(agfx_renderer_t *renderer);
static agfx_result_t create_texture_sampler(agfx_renderer_t *renderer);

static void free_descriptor_set_layout(agfx_renderer_t *renderer);
static void free_descriptor_pool(agfx_renderer_t *renderer);
static void free_descriptor_sets(agfx_renderer_t *renderer);
static void free_pipeline(agfx_renderer_t *renderer);
static void free_render_pass(agfx_renderer_t *renderer);
static void free_command_pool(agfx_renderer_t *renderer);
static void free_command_buffers(agfx_renderer_t *renderer);
static void free_sync_objects(agfx_renderer_t *renderer);
static void free_vertex_buffer(agfx_renderer_t *renderer);
static void free_index_buffer(agfx_renderer_t *renderer);
static void free_uniform_buffers(agfx_renderer_t *renderer);
static void free_texture_image(agfx_renderer_t *renderer);
static void free_texture_image_view(agfx_renderer_t *renderer);
static void free_texture_sampler(agfx_renderer_t *renderer);

void agfx_update_uniform_buffer(agfx_renderer_t *renderer);
agfx_result_t agfx_create_renderer(agfx_context_t* context, agfx_swapchain_t* swapchain, agfx_state_t* state, agfx_renderer_t* out_renderer);
void agfx_free_renderer(agfx_renderer_t *renderer);
agfx_result_t agfx_record_command_buffers(agfx_renderer_t *renderer, uint32_t image_index);

#endif