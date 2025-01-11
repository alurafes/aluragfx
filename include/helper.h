#ifndef AGFX_VULKAN_HELPER_H
#define AGFX_VULKAN_HELPER_H

#include "engine_types.h"

agfx_result_t agfx_helper_command_buffer_end(agfx_renderer_t *renderer, VkCommandBuffer *command_buffer);
agfx_result_t agfx_helper_command_buffer_begin(agfx_renderer_t *renderer, VkCommandBuffer* command_buffer);
agfx_result_t agfx_helper_create_buffer(agfx_context_t *context, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
agfx_result_t agfx_helper_create_image(agfx_context_t *context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkImage* image, VkDeviceMemory* image_memory);
agfx_result_t agfx_helper_create_image_view(agfx_context_t *context, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView *image_view);
agfx_result_t agfx_helper_copy_buffer(agfx_renderer_t *renderer, VkBuffer src, VkBuffer dst, VkDeviceSize size);
agfx_result_t agfx_helper_copy_buffer_to_image(agfx_renderer_t *renderer, VkBuffer src, VkImage dst, uint32_t width, uint32_t height);
agfx_result_t agfx_helper_transition_image_layout(agfx_renderer_t *renderer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

#endif