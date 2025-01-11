#ifndef AGFX_ENGINE_TYPES_H
#define AGFX_ENGINE_TYPES_H

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include "math/vector.h"
#include "math/martix.h"

typedef enum agfx_result_t {
    AGFX_SUCCESS = 0,
    AGFX_WINDOW_CREATE_ERROR,
    AGFX_VULKAN_EXTENSIONS_ERROR,
    AGFX_VULKAN_INSTANCE_ERROR,
    AGFX_VULKAN_SURFACE_ERROR,
    AGFX_NO_PHYSICAL_DEVICE_ERROR,
    AGFX_QUEUE_FAMILY_INDICES_ERROR,
    AGFX_DEVICE_CREATE_ERROR,
    AGFX_SURFACE_CAPABILITIES_ERROR,
    AGFX_SURFACE_FORMATS_ERROR,
    AGFX_SURFACE_PRESENT_MODES_ERROR,
    AGFX_SWAPCHAIN_ERROR,
    AGFX_SWAPCHAIN_IMAGE_VIEW_ERROR,
    AGFX_RENDER_PASS_ERROR,
    AGFX_FRAMEBUFFER_ERROR,
    AGFX_COMMAND_POOL_ERROR,
    AGFX_COMMAND_BUFFERS_ERROR,
    AGFX_PIPELINE_ERROR,
    AGFX_SYNC_OBJECT_ERROR,
    AGFX_VERTEX_BUFFER_ERROR,
    AGFX_INDEX_BUFFER_ERROR,
    AGFX_BUFFER_ERROR,
    AGFX_BUFFER_COPY_ERROR,
    AGFX_BUFFER_MAP_ERROR,
    AGFX_DESCRIPTOR_SET_LAYOUT_ERROR,
    AGFX_DESCRIPTOR_POOL_ERROR,
    AGFX_DESCRIPTOR_SET_ERROR,
    AGFX_IMAGE_LOAD_ERROR,
    AGFX_IMAGE_CREATE_ERROR,
    AGFX_UNSUPPORTED_LAYOUT_TRANSITION_ERROR,
    AGFX_SAMPLER_CREATE_ERROR,
} agfx_result_t;

#define AGFX_QUEUE_FAMILY_INDICES_LENGTH sizeof(agfx_queue_family_indices_t) / sizeof(uint32_t)
typedef struct agfx_queue_family_indices_t {
    uint32_t graphics_index;
    uint32_t present_index;
} agfx_queue_family_indices_t;

typedef struct agfx_swapchain_info_t {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t surface_formats_count;
    VkSurfaceFormatKHR* surface_formats;
    uint32_t present_modes_count;
    VkPresentModeKHR* present_modes;
} agfx_swapchain_info_t;

typedef struct agfx_present_t {
    SDL_Window* window;
    uint32_t width, height;
} agfx_present_t;

typedef struct agfx_context_t {
    agfx_present_t* present;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSurfaceKHR surface;
    agfx_queue_family_indices_t queue_family_indices;
} agfx_context_t;

typedef struct agfx_renderer_t agfx_renderer_t;

typedef struct agfx_swapchain_t {
    agfx_context_t* context;
    agfx_present_t* present;
    agfx_renderer_t* renderer;
    agfx_swapchain_info_t swapchain_info;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_images_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;
    VkFramebuffer* framebuffers;
} agfx_swapchain_t;

typedef struct agfx_state_t {
    uint32_t quit;
    uint32_t resized;
    uint32_t current_frame;
    agfx_vector3_t rotation;
} agfx_state_t;

typedef struct agfx_renderer_t {
    agfx_context_t* context;
    agfx_swapchain_t* swapchain;
    agfx_state_t* state;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    VkPipeline pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;
    VkBuffer* uniform_buffers;
    VkDeviceMemory* uniform_buffer_memories;
    void** uniform_buffer_mapped;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
    VkImageView texture_image_view;
    VkSampler texture_sampler;
} agfx_renderer_t;

typedef struct agfx_engine_t {
    agfx_present_t present;
    agfx_context_t context;
    agfx_swapchain_t swapchain;
    agfx_renderer_t renderer;
    agfx_state_t state;
} agfx_engine_t;

typedef struct agfx_uniform_buffer_object_t {
    agfx_mat4x4_t model;
    agfx_mat4x4_t view;
    agfx_mat4x4_t projection;
} agfx_uniform_buffer_object_t;

typedef struct agfx_vertex_t {
    agfx_vector3_t position;
    agfx_vector3_t color;
    agfx_vector2_t texture_coordinate;
} agfx_vertex_t;

#endif