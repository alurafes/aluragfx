#include "swapchain.h"

#include <stdio.h>

agfx_result_t populate_swapchain_info(agfx_swapchain_t *swapchain)
{
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapchain->context->physical_device, swapchain->context->surface, &swapchain->swapchain_info.surface_capabilities))
    {
        return AGFX_SURFACE_CAPABILITIES_ERROR;
    }
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(swapchain->context->physical_device, swapchain->context->surface, &swapchain->swapchain_info.surface_formats_count, NULL))
    {
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    if (swapchain->swapchain_info.surface_formats_count == 0)
    {
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    swapchain->swapchain_info.surface_formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * swapchain->swapchain_info.surface_formats_count);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(swapchain->context->physical_device, swapchain->context->surface, &swapchain->swapchain_info.surface_formats_count, swapchain->swapchain_info.surface_formats))
    {
        free(swapchain->swapchain_info.surface_formats);
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(swapchain->context->physical_device, swapchain->context->surface, &swapchain->swapchain_info.present_modes_count, NULL))
    {
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    if (swapchain->swapchain_info.present_modes_count == 0)
    {
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    swapchain->swapchain_info.present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * swapchain->swapchain_info.present_modes_count);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(swapchain->context->physical_device, swapchain->context->surface, &swapchain->swapchain_info.present_modes_count, swapchain->swapchain_info.present_modes))
    {
        free(swapchain->swapchain_info.surface_formats);
        free(swapchain->swapchain_info.present_modes);
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    
    return AGFX_SUCCESS;
}

agfx_result_t create_swapchain(agfx_swapchain_t *swapchain)
{
    VkSurfaceFormatKHR surface_format = swapchain->swapchain_info.surface_formats[0];
    for (int i = 0; i < swapchain->swapchain_info.surface_formats_count; ++i)
    {
        if (swapchain->swapchain_info.surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && swapchain->swapchain_info.surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = swapchain->swapchain_info.surface_formats[i];
            break;
        }
    }
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (int i = 0; i < swapchain->swapchain_info.present_modes_count; ++i)
    {
        if (swapchain->swapchain_info.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = swapchain->swapchain_info.present_modes[i];
            break;
        }
    }
    VkExtent2D extent = swapchain->swapchain_info.surface_capabilities.currentExtent;
    if (swapchain->swapchain_info.surface_capabilities.currentExtent.width == UINT32_MAX)
    {
        int width, height;
        SDL_Vulkan_GetDrawableSize(swapchain->present->window, &width, &height);
        extent.width = fmin(swapchain->swapchain_info.surface_capabilities.maxImageExtent.width, fmax(width, swapchain->swapchain_info.surface_capabilities.minImageExtent.width));
        extent.height = fmin(swapchain->swapchain_info.surface_capabilities.maxImageExtent.height, fmax(height, swapchain->swapchain_info.surface_capabilities.minImageExtent.height));
    }

    uint32_t image_count = swapchain->swapchain_info.surface_capabilities.minImageCount + 1;
    if (swapchain->swapchain_info.surface_capabilities.maxImageCount != 0 && image_count > swapchain->swapchain_info.surface_capabilities.maxImageCount)
    {
        image_count = swapchain->swapchain_info.surface_capabilities.maxImageCount;
    }

    uint32_t queue_family_indices_count = 0;
    uint32_t queue_family_indices[2] = {swapchain->context->queue_family_indices.graphics_index, swapchain->context->queue_family_indices.present_index};

    VkSharingMode sharing_mode = swapchain->context->queue_family_indices.graphics_index != swapchain->context->queue_family_indices.present_index ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    if (sharing_mode == VK_SHARING_MODE_CONCURRENT)
    {
        queue_family_indices_count = 2;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain->context->surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .pQueueFamilyIndices = queue_family_indices,
        .queueFamilyIndexCount = queue_family_indices_count,
        .preTransform = swapchain->swapchain_info.surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = 0
    };

    if (VK_SUCCESS != vkCreateSwapchainKHR(swapchain->context->device, &swapchain_create_info, NULL, &swapchain->swapchain))
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(swapchain->context->device, swapchain->swapchain, &swapchain->swapchain_images_count, NULL))
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    if (swapchain->swapchain_images_count == 0)
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    swapchain->swapchain_images = (VkImage*)malloc(sizeof(VkImage) * swapchain->swapchain_images_count);
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(swapchain->context->device, swapchain->swapchain, &swapchain->swapchain_images_count, swapchain->swapchain_images))
    {
        free(swapchain->swapchain_images);
        return AGFX_SWAPCHAIN_ERROR;
    }

    swapchain->swapchain_format = surface_format.format;
    swapchain->swapchain_extent = extent;

    return AGFX_SUCCESS;
}

agfx_result_t create_swapchain_image_views(agfx_swapchain_t *swapchain)
{
    swapchain->swapchain_image_views = (VkImageView*)malloc(sizeof(VkImageView) * swapchain->swapchain_images_count);
    for (int i = 0; i < swapchain->swapchain_images_count; ++i)
    {
        VkComponentMapping components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        };
        VkImageSubresourceRange subresource_range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->swapchain_format,
            .components = components,
            .subresourceRange = subresource_range,
            .image = swapchain->swapchain_images[i],
        };
        if (VK_SUCCESS != vkCreateImageView(swapchain->context->device, &image_view_create_info, NULL, &swapchain->swapchain_image_views[i]))
        {
            free(swapchain->swapchain_image_views);
            return AGFX_SWAPCHAIN_IMAGE_VIEW_ERROR;
        }
    }
    return AGFX_SUCCESS;
}

agfx_result_t create_framebuffers(agfx_swapchain_t *swapchain)
{
    swapchain->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchain->swapchain_images_count);
    for (int i = 0; i < swapchain->swapchain_images_count; ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = swapchain->renderer->render_pass,
            .width = swapchain->swapchain_extent.width,
            .height = swapchain->swapchain_extent.height,
            .layers = 1,
            .attachmentCount = 1,
            .pAttachments = &swapchain->swapchain_image_views[i]
        };

        if (VK_SUCCESS != vkCreateFramebuffer(swapchain->context->device, &framebuffer_create_info, NULL, &swapchain->framebuffers[i]))
        {
            free(swapchain->framebuffers);
            return AGFX_FRAMEBUFFER_ERROR;
        }
    }
    return AGFX_SUCCESS;
}


void free_swapchain_info(agfx_swapchain_t* swapchain)
{
    free(swapchain->swapchain_info.surface_formats);
    free(swapchain->swapchain_info.present_modes);
}

void free_swapchain(agfx_swapchain_t* swapchain)
{
    free(swapchain->swapchain_images);
    vkDestroySwapchainKHR(swapchain->context->device, swapchain->swapchain, NULL);
}

void free_swapchain_image_views(agfx_swapchain_t* swapchain)
{
    for (int i = 0; i < swapchain->swapchain_images_count; ++i)
    {
        vkDestroyImageView(swapchain->context->device, swapchain->swapchain_image_views[i], NULL);
    }
    free(swapchain->swapchain_image_views);
}

void free_framebuffers(agfx_swapchain_t* swapchain)
{
    for (int i = 0; i < swapchain->swapchain_images_count; ++i)
    {
        vkDestroyFramebuffer(swapchain->context->device, swapchain->framebuffers[i], NULL);
    }
    free(swapchain->framebuffers);
}

agfx_result_t agfx_create_swapchain(agfx_context_t* context, agfx_present_t* present, agfx_swapchain_t* out_swapchain)
{
    agfx_swapchain_t swapchain;
    agfx_result_t result;
    
    swapchain.context = context;
    swapchain.present = present;

    result = populate_swapchain_info(&swapchain);
    if (AGFX_SUCCESS != result) goto finish;

    result = create_swapchain(&swapchain);
    if (AGFX_SUCCESS != result) goto free_swapchain_info;

    result = create_swapchain_image_views(&swapchain);
    if (AGFX_SUCCESS != result) goto free_swapchain;
    
goto finish;

free_swapchain:
    free_swapchain(&swapchain);
free_swapchain_info:
    free_swapchain_info(&swapchain);
finish:
    *out_swapchain = swapchain;
    return result;
}

void agfx_free_swapchain(agfx_swapchain_t* swapchain)
{
    free_framebuffers(swapchain);
    free_swapchain_image_views(swapchain);
    free_swapchain(swapchain);
    free_swapchain_info(swapchain);
}

agfx_result_t agfx_recreate_swapchain(agfx_swapchain_t *swapchain)
{
    vkDeviceWaitIdle(swapchain->context->device);
    
    free_framebuffers(swapchain);
    free_swapchain_image_views(swapchain);
    free_swapchain(swapchain);

    agfx_result_t result = AGFX_SUCCESS;

    result = populate_swapchain_info(swapchain);
    if (AGFX_SUCCESS != result) return result;

    result = create_swapchain(swapchain);
    if (AGFX_SUCCESS != result) return result;

    result = create_swapchain_image_views(swapchain);
    if (AGFX_SUCCESS != result) {
        free_swapchain(swapchain);
        return result;
    }

    result = create_framebuffers(swapchain);
    if (AGFX_SUCCESS != result) {
        free_swapchain_image_views(swapchain);
        free_swapchain(swapchain);
    }

    return result;
}

agfx_result_t agfx_create_framebuffers(agfx_swapchain_t *swapchain)
{
    return create_framebuffers(swapchain);
}