#include "helper.h"

uint32_t find_vulkan_memory_type(agfx_context_t *context, uint32_t type_filter, VkMemoryPropertyFlags property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context->physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }
}

agfx_result_t agfx_helper_create_image(agfx_context_t *context, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkImage* image, VkDeviceMemory* image_memory)
{
    agfx_result_t result = AGFX_SUCCESS;
    VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .flags = 0
    };

    if (VK_SUCCESS != vkCreateImage(context->device, &image_create_info, NULL, image))
    {
        result = AGFX_IMAGE_CREATE_ERROR;
        return result;
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(context->device, *image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_vulkan_memory_type(context, memory_requirements.memoryTypeBits, property_flags)
    };

    if (VK_SUCCESS != vkAllocateMemory(context->device, &memory_allocate_info, NULL, image_memory)) 
    {
        result = AGFX_BUFFER_ERROR;
        return result;
    }

    if (VK_SUCCESS != vkBindImageMemory(context->device, *image, *image_memory, 0))
    {
        result = AGFX_BUFFER_ERROR;
        vkFreeMemory(context->device, *image_memory, NULL);
        return result;
    }

    return result;
}

agfx_result_t agfx_helper_command_buffer_begin(agfx_renderer_t *renderer, VkCommandBuffer* command_buffer)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = renderer->command_pool,
        .commandBufferCount = 1
    };

    if (VK_SUCCESS != vkAllocateCommandBuffers(renderer->context->device, &command_buffer_allocate_info, command_buffer))
    {
        return AGFX_BUFFER_COPY_ERROR;
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (VK_SUCCESS != vkBeginCommandBuffer(*command_buffer, &begin_info))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t agfx_helper_command_buffer_end(agfx_renderer_t *renderer, VkCommandBuffer *command_buffer)
{
    vkEndCommandBuffer(*command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = command_buffer
    };

    if (VK_SUCCESS != vkQueueSubmit(renderer->context->graphics_queue, 1, &submit_info, 0))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    if (VK_SUCCESS != vkQueueWaitIdle(renderer->context->graphics_queue))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, command_buffer);

    return AGFX_SUCCESS;
}

agfx_result_t agfx_helper_copy_buffer(agfx_renderer_t *renderer, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    agfx_result_t result = AGFX_SUCCESS;
    VkCommandBuffer command_buffer;
    result = agfx_helper_command_buffer_begin(renderer, &command_buffer);
    if (AGFX_SUCCESS != result) return result;

    VkBufferCopy buffer_copy = {
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, src, dst, 1, &buffer_copy);

    result = agfx_helper_command_buffer_end(renderer, &command_buffer);
    return result;
}

agfx_result_t agfx_helper_copy_buffer_to_image(agfx_renderer_t *renderer, VkBuffer src, VkImage dst, uint32_t width, uint32_t height)
{
    agfx_result_t result = AGFX_SUCCESS;
    VkCommandBuffer command_buffer;
    result = agfx_helper_command_buffer_begin(renderer, &command_buffer);
    if (AGFX_SUCCESS != result) return result;

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {
            .x = 0, .y = 0, .z = 0
        },
        .imageExtent = {
            .depth = 1,
            .width = width,
            .height = height
        }
    };

    vkCmdCopyBufferToImage(command_buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    result = agfx_helper_command_buffer_end(renderer, &command_buffer);
    return result;
}

agfx_result_t agfx_helper_create_buffer(agfx_context_t *context, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (VK_SUCCESS != vkCreateBuffer(context->device, &buffer_create_info, NULL, buffer))
    {
        return AGFX_BUFFER_ERROR;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(context->device, *buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = find_vulkan_memory_type(context, memory_requirements.memoryTypeBits, property_flags),
        .allocationSize = memory_requirements.size
    };

    if (VK_SUCCESS != vkAllocateMemory(context->device, &memory_allocate_info, NULL, buffer_memory))
    {
        vkDestroyBuffer(context->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }

    if (VK_SUCCESS != vkBindBufferMemory(context->device, *buffer, *buffer_memory, 0))
    {
        vkFreeMemory(context->device, *buffer_memory, NULL);
        vkDestroyBuffer(context->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }
}

agfx_result_t agfx_helper_create_image_view(agfx_context_t *context, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView *image_view)
{
    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = aspect_flags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

    if (VK_SUCCESS != vkCreateImageView(context->device, &image_view_create_info, NULL, image_view))
    {
        return AGFX_SWAPCHAIN_IMAGE_VIEW_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t agfx_helper_transition_image_layout(agfx_renderer_t* renderer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    agfx_result_t result = AGFX_SUCCESS;

    VkCommandBuffer command_buffer;
    result = agfx_helper_command_buffer_begin(renderer, &command_buffer);
    if (AGFX_SUCCESS != result) return result;

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseArrayLayer = 0,
            .layerCount = 1,
            .baseMipLevel = 0,
            .levelCount = 1
        },
        .srcAccessMask = 0,
        .dstAccessMask = 0
    };

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else
    {
        result = agfx_helper_command_buffer_end(renderer, &command_buffer);
        return AGFX_UNSUPPORTED_LAYOUT_TRANSITION_ERROR;
    }

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

    result = agfx_helper_command_buffer_end(renderer, &command_buffer);
    return result;
}
