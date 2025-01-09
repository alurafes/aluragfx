#include "renderer.h"

#include "math/martix.h"

agfx_result_t create_render_pass(agfx_renderer_t *renderer)
{
    VkAttachmentDescription attachment_description = {
        .format = renderer->swapchain->swapchain_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference attachment_description_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass_description = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = &attachment_description_ref,
        .colorAttachmentCount = 1,
    };

    VkSubpassDependency subpass_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo render_pass_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment_description,
        .subpassCount = 1,
        .pSubpasses = &subpass_description,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency
    };

    if (VK_SUCCESS != vkCreateRenderPass(renderer->context->device, &render_pass_create_info, NULL, &renderer->render_pass))
    {
        return AGFX_RENDER_PASS_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t create_command_pool(agfx_renderer_t *renderer)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = renderer->context->queue_family_indices.graphics_index,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };

    if (VK_SUCCESS != vkCreateCommandPool(renderer->context->device, &command_pool_create_info, NULL, &renderer->command_pool))
    {
        return AGFX_COMMAND_POOL_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t create_command_buffers(agfx_renderer_t *renderer)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = renderer->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = AGFX_MAX_FRAMES_IN_FLIGHT,
    };

    renderer->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * AGFX_MAX_FRAMES_IN_FLIGHT);

    if (VK_SUCCESS != vkAllocateCommandBuffers(renderer->context->device, &command_buffer_allocate_info, renderer->command_buffers))
    {
        free(renderer->command_buffers);
        return AGFX_COMMAND_BUFFERS_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t agfx_record_command_buffers(agfx_renderer_t *renderer, uint32_t image_index)
{
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = {};
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pInheritanceInfo = &command_buffer_inheritance_info,
        .flags = 0,
    };

    if (VK_SUCCESS != vkBeginCommandBuffer(renderer->command_buffers[renderer->state->current_frame], &command_buffer_begin_info))
    {
        return AGFX_COMMAND_BUFFERS_ERROR;
    }

    VkRect2D render_area = {
        .offset = {},
        .extent = renderer->swapchain->swapchain_extent,
    };


    VkClearValue clear_value = {
        .color = {
            .float32 = {0.0, 0.0, 0.0, 1.0}
        }
    };

    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderer->render_pass,
        .framebuffer = renderer->swapchain->framebuffers[image_index],
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    vkCmdBeginRenderPass(renderer->command_buffers[renderer->state->current_frame], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(renderer->command_buffers[renderer->state->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline);
    vkCmdBindVertexBuffers(renderer->command_buffers[renderer->state->current_frame], 0, 1, &renderer->vertex_buffer, &(VkDeviceSize){0});
    vkCmdBindIndexBuffer(renderer->command_buffers[renderer->state->current_frame], renderer->index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(renderer->command_buffers[renderer->state->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout, 0, 1, &renderer->descriptor_sets[renderer->state->current_frame], 0, NULL);
    vkCmdDrawIndexed(renderer->command_buffers[renderer->state->current_frame], AGFX_INDEX_ARRAY_SIZE, 1, 0, 0, 0);

    vkCmdEndRenderPass(renderer->command_buffers[renderer->state->current_frame]);
    vkEndCommandBuffer(renderer->command_buffers[renderer->state->current_frame]);
    
    return AGFX_SUCCESS;
}

agfx_result_t create_pipeline(agfx_renderer_t *renderer)
{
    const char* vertex_shader_file_path = "./shaders/vert.spv";
    struct stat vertex_shader_stat;
    stat(vertex_shader_file_path, &vertex_shader_stat);
    char* vertex_shader_data = (char*)malloc(sizeof(char) * vertex_shader_stat.st_size);
    FILE *vertex_shader_fd = fopen(vertex_shader_file_path, "rb");
    fread(vertex_shader_data, vertex_shader_stat.st_size, 1, vertex_shader_fd);
    fclose(vertex_shader_fd);

    VkShaderModuleCreateInfo vert_shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vertex_shader_stat.st_size,
        .pCode = (uint32_t*)vertex_shader_data
    };

    VkShaderModule vert_shader_module;
    if (VK_SUCCESS != vkCreateShaderModule(renderer->context->device, &vert_shader_module_create_info, NULL, &vert_shader_module))
    {
        free(vertex_shader_data);
        return AGFX_PIPELINE_ERROR;
    }

    const char* fragment_shader_file_path = "./shaders/frag.spv";
    struct stat fragment_shader_stat;
    stat(fragment_shader_file_path, &fragment_shader_stat);
    char* fragment_shader_data = (char*)malloc(fragment_shader_stat.st_size);
    FILE *fragment_shader_fd = fopen(fragment_shader_file_path, "rb");
    fread(fragment_shader_data, fragment_shader_stat.st_size, 1, fragment_shader_fd);
    fclose(fragment_shader_fd);

    VkShaderModuleCreateInfo frag_shader_module_create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fragment_shader_stat.st_size,
        .pCode = (uint32_t*)fragment_shader_data
    };

    VkShaderModule frag_shader_module;
    if (VK_SUCCESS != vkCreateShaderModule(renderer->context->device, &frag_shader_module_create_info, NULL, &frag_shader_module))
    {
        free(vertex_shader_data);
        free(fragment_shader_data);
        return AGFX_PIPELINE_ERROR;
    }

    free(vertex_shader_data);
    free(fragment_shader_data);

    VkPipelineShaderStageCreateInfo vert_shader_stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo frag_shader_stage_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo stages[] = {
        vert_shader_stage_create_info,
        frag_shader_stage_create_info
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &agfx_vertex_input_binding_description,
        .vertexAttributeDescriptionCount = AGFX_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_SIZE,
        .pVertexAttributeDescriptions = agfx_vertex_input_attribute_description
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)renderer->swapchain->swapchain_extent.width,
        .height = (float)renderer->swapchain->swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .extent = renderer->swapchain->swapchain_extent,
        .offset = {0, 0}
    };

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL, // FUN
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .blendEnable = VK_TRUE, 
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &renderer->descriptor_set_layout
    };

    if (VK_SUCCESS != vkCreatePipelineLayout(renderer->context->device, &pipeline_layout_create_info, NULL, &renderer->pipeline_layout))
    {
        vkDestroyShaderModule(renderer->context->device, vert_shader_module, NULL);
        vkDestroyShaderModule(renderer->context->device, frag_shader_module, NULL);
        free(vertex_shader_data);
        free(fragment_shader_data);
        return AGFX_PIPELINE_ERROR;
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .layout = renderer->pipeline_layout,
        .renderPass = renderer->render_pass,
        .subpass = 0,
        .pViewportState = &viewport_state_create_info,
    };

    if (VK_SUCCESS != vkCreateGraphicsPipelines(renderer->context->device, 0, 1, &pipeline_create_info, NULL, &renderer->pipeline))
    {
        vkDestroyShaderModule(renderer->context->device, vert_shader_module, NULL);
        vkDestroyShaderModule(renderer->context->device, frag_shader_module, NULL);
        free(vertex_shader_data);
        free(fragment_shader_data);
        return AGFX_PIPELINE_ERROR;
    }

    vkDestroyShaderModule(renderer->context->device, vert_shader_module, NULL);
    vkDestroyShaderModule(renderer->context->device, frag_shader_module, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_sync_objects(agfx_renderer_t *renderer)
{
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    renderer->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * AGFX_MAX_FRAMES_IN_FLIGHT);
    renderer->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * AGFX_MAX_FRAMES_IN_FLIGHT);
    renderer->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * AGFX_MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(renderer->context->device, &semaphore_create_info, NULL, &renderer->image_available_semaphores[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }

        if (VK_SUCCESS != vkCreateSemaphore(renderer->context->device, &semaphore_create_info, NULL, &renderer->render_finished_semaphores[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }

        if (VK_SUCCESS != vkCreateFence(renderer->context->device, &fence_create_info, NULL, &renderer->in_flight_fences[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }
    }

    return AGFX_SUCCESS;
}


uint32_t find_vulkan_memory_type(agfx_renderer_t *renderer, uint32_t type_filter, VkMemoryPropertyFlags property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(renderer->context->physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }
}

agfx_result_t create_buffer(agfx_renderer_t *renderer, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (VK_SUCCESS != vkCreateBuffer(renderer->context->device, &buffer_create_info, NULL, buffer))
    {
        return AGFX_BUFFER_ERROR;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(renderer->context->device, *buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = find_vulkan_memory_type(renderer, memory_requirements.memoryTypeBits, property_flags),
        .allocationSize = memory_requirements.size
    };

    if (VK_SUCCESS != vkAllocateMemory(renderer->context->device, &memory_allocate_info, NULL, buffer_memory))
    {
        vkDestroyBuffer(renderer->context->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }

    if (VK_SUCCESS != vkBindBufferMemory(renderer->context->device, *buffer, *buffer_memory, 0))
    {
        vkFreeMemory(renderer->context->device, *buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }
}

agfx_result_t copy_buffer(agfx_renderer_t *renderer, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = renderer->command_pool,
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;
    if (VK_SUCCESS != vkAllocateCommandBuffers(renderer->context->device, &command_buffer_allocate_info, &command_buffer))
    {
        return AGFX_BUFFER_COPY_ERROR;
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (VK_SUCCESS != vkBeginCommandBuffer(command_buffer, &begin_info))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, &command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    VkBufferCopy buffer_copy = {
        .size = size
    };

    vkCmdCopyBuffer(command_buffer, src, dst, 1, &buffer_copy);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    if (VK_SUCCESS != vkQueueSubmit(renderer->context->graphics_queue, 1, &submit_info, 0))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, &command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    if (VK_SUCCESS != vkQueueWaitIdle(renderer->context->graphics_queue))
    {
        vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, &command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, 1, &command_buffer);
    return AGFX_SUCCESS;
}

agfx_result_t create_vertex_buffer(agfx_renderer_t *renderer)
{
    agfx_result_t result;
    size_t vertex_buffer_size = sizeof(agfx_vertices[0]) * AGFX_VERTEX_ARRAY_SIZE;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    result = create_buffer(renderer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        return result;
    }

    void* buffer;
    if (VK_SUCCESS != vkMapMemory(renderer->context->device, staging_buffer_memory, 0, vertex_buffer_size, 0, &buffer))
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return AGFX_VERTEX_BUFFER_ERROR;
    }
    memcpy(buffer, agfx_vertices, vertex_buffer_size);
    vkUnmapMemory(renderer->context->device, staging_buffer_memory);

    result = create_buffer(renderer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderer->vertex_buffer, &renderer->vertex_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return result;
    }

    result = copy_buffer(renderer, staging_buffer, renderer->vertex_buffer, vertex_buffer_size);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, renderer->vertex_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, renderer->vertex_buffer, NULL);
        return result;
    }

    vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
    vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_index_buffer(agfx_renderer_t *renderer)
{
    agfx_result_t result;
    size_t index_buffer_size = sizeof(agfx_indices[0]) * AGFX_INDEX_ARRAY_SIZE;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    result = create_buffer(renderer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        return result;
    }

    void* buffer;
    if (VK_SUCCESS != vkMapMemory(renderer->context->device, staging_buffer_memory, 0, index_buffer_size, 0, &buffer))
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return AGFX_INDEX_BUFFER_ERROR;
    }
    memcpy(buffer, agfx_indices, index_buffer_size);
    vkUnmapMemory(renderer->context->device, staging_buffer_memory);

    result = create_buffer(renderer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderer->index_buffer, &renderer->index_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return result;
    }

    result = copy_buffer(renderer, staging_buffer, renderer->index_buffer, index_buffer_size);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, renderer->index_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, renderer->index_buffer, NULL);
        return result;
    }

    vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
    vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_descriptor_set_layout(agfx_renderer_t *renderer)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .binding = 0,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };

    VkDescriptorSetLayoutCreateInfo ubo_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &ubo_layout_binding
    };

    if (VK_SUCCESS != vkCreateDescriptorSetLayout(renderer->context->device, &ubo_layout_create_info, NULL, &renderer->descriptor_set_layout))
    {
        return AGFX_DESCRIPTOR_SET_LAYOUT_ERROR;
    }

}

void free_descriptor_set_layout(agfx_renderer_t *renderer) 
{
    vkDestroyDescriptorSetLayout(renderer->context->device, renderer->descriptor_set_layout, NULL);
}

void free_pipeline(agfx_renderer_t *renderer) 
{
    vkDestroyPipeline(renderer->context->device, renderer->pipeline, NULL);
    vkDestroyPipelineLayout(renderer->context->device, renderer->pipeline_layout, NULL);
}

void free_render_pass(agfx_renderer_t *renderer)
{
    vkDestroyRenderPass(renderer->context->device, renderer->render_pass, NULL);
}

void free_command_pool(agfx_renderer_t *renderer)
{
    vkDestroyCommandPool(renderer->context->device, renderer->command_pool, NULL);
}

void free_command_buffers(agfx_renderer_t *renderer)
{
    vkFreeCommandBuffers(renderer->context->device, renderer->command_pool, AGFX_MAX_FRAMES_IN_FLIGHT, renderer->command_buffers);
}

void free_sync_objects(agfx_renderer_t *renderer)
{
    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(renderer->context->device, renderer->image_available_semaphores[i], NULL);
        vkDestroySemaphore(renderer->context->device, renderer->render_finished_semaphores[i], NULL);
        vkDestroyFence(renderer->context->device, renderer->in_flight_fences[i], NULL);
    }
}

void free_vertex_buffer(agfx_renderer_t *renderer)
{
    vkDestroyBuffer(renderer->context->device, renderer->vertex_buffer, NULL);
    vkFreeMemory(renderer->context->device, renderer->vertex_buffer_memory, NULL);
}

void free_index_buffer(agfx_renderer_t *renderer)
{
    vkDestroyBuffer(renderer->context->device, renderer->index_buffer, NULL);
    vkFreeMemory(renderer->context->device, renderer->index_buffer_memory, NULL);
}

agfx_result_t agfx_create_renderer(agfx_context_t* context, agfx_swapchain_t* swapchain, agfx_state_t* state, agfx_renderer_t* out_renderer)
{
    agfx_renderer_t renderer;
    agfx_result_t result;

    renderer.context = context;
    renderer.swapchain = swapchain;
    renderer.state = state;

    result = create_descriptor_set_layout(&renderer);
    if (AGFX_SUCCESS != result) goto finish;

    result = create_render_pass(&renderer);
    if (AGFX_SUCCESS != result) goto free_descriptor_set_layout;

    result = create_pipeline(&renderer);
    if (AGFX_SUCCESS != result) goto free_render_pass;

    result = create_command_pool(&renderer);
    if (AGFX_SUCCESS != result) goto free_pipeline;

    result = create_vertex_buffer(&renderer);
    if (AGFX_SUCCESS != result) goto free_command_pool;
    
    result = create_index_buffer(&renderer);
    if (AGFX_SUCCESS != result) goto free_vertex_buffer;

    result = create_uniform_buffers(&renderer);
    if (AGFX_SUCCESS != result) goto free_index_buffer;

    result = create_descriptor_pool(&renderer);
    if (AGFX_SUCCESS != result) goto free_uniform_buffers;

    result = create_descriptor_set(&renderer);
    if (AGFX_SUCCESS != result) goto free_descriptor_pool;

    result = create_command_buffers(&renderer);
    if (AGFX_SUCCESS != result) goto free_descriptor_set;

    result = create_sync_objects(&renderer);
    if (AGFX_SUCCESS != result) goto free_command_buffers;

goto finish;

free_sync_objects:
    free_sync_objects(&renderer);
free_command_buffers:
    free_command_buffers(&renderer);
free_descriptor_set:
    free_descriptor_set(&renderer);
free_descriptor_pool:
    free_descriptor_pool(&renderer);
free_uniform_buffers:
    free_uniform_buffers(&renderer);
free_index_buffer:
    free_index_buffer(&renderer);
free_vertex_buffer:
    free_vertex_buffer(&renderer);
free_command_pool:
    free_command_pool(&renderer);
free_render_pass:
    free_render_pass(&renderer);
free_pipeline:
    free_pipeline(&renderer);
free_descriptor_set_layout:
    free_descriptor_set_layout(&renderer);
finish:
    *out_renderer = renderer;
    return result;
}

void agfx_free_renderer(agfx_renderer_t *renderer)
{
    free_sync_objects(renderer);
    free_command_buffers(renderer);
    free_descriptor_set(renderer);
    free_descriptor_pool(renderer);
    free_uniform_buffers(renderer);
    free_index_buffer(renderer);
    free_vertex_buffer(renderer);
    free_command_pool(renderer);
    free_render_pass(renderer);
    free_pipeline(renderer);
    free_descriptor_set_layout(renderer);
}

// todo: refactor
// todo: malloc result checks
agfx_result_t create_uniform_buffers(agfx_renderer_t *renderer)
{
    renderer->uniform_buffers = malloc(sizeof(VkBuffer) * AGFX_MAX_FRAMES_IN_FLIGHT);
    renderer->uniform_buffer_memories = malloc(sizeof(VkDeviceMemory) * AGFX_MAX_FRAMES_IN_FLIGHT);
    renderer->uniform_buffer_mapped = malloc(sizeof(void*) * AGFX_MAX_FRAMES_IN_FLIGHT);

    agfx_result_t result = AGFX_SUCCESS;
    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        result = create_buffer(
            renderer, 
            sizeof(agfx_uniform_buffer_object_t), 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &renderer->uniform_buffers[i], 
            &renderer->uniform_buffer_memories[i]
        );

        if (AGFX_SUCCESS != result)
        {
            goto free;
        }

        if (VK_SUCCESS != vkMapMemory(renderer->context->device, renderer->uniform_buffer_memories[i], 0, sizeof(agfx_uniform_buffer_object_t), 0, &renderer->uniform_buffer_mapped[i]))
        {
            result = AGFX_BUFFER_MAP_ERROR;
        }

        if (result == AGFX_SUCCESS) goto finish;

        free:
        if (i > 0) {
            for (size_t j = 0; j < i; ++j)
            {
                vkDestroyBuffer(renderer->context->device, renderer->uniform_buffers[j], NULL);
                vkFreeMemory(renderer->context->device, renderer->uniform_buffer_memories[j], NULL);
            }
        }
        free(renderer->uniform_buffers);
        free(renderer->uniform_buffer_memories);
        free(renderer->uniform_buffer_mapped);
        return result;

        finish:
    }
    return result;
}

void free_uniform_buffers(agfx_renderer_t *renderer)
{
    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyBuffer(renderer->context->device, renderer->uniform_buffers[i], NULL);
        vkFreeMemory(renderer->context->device, renderer->uniform_buffer_memories[i], NULL);
    }

    free(renderer->uniform_buffers);
    free(renderer->uniform_buffer_memories);
    free(renderer->uniform_buffer_mapped);
}

void agfx_update_uniform_buffer(agfx_renderer_t *renderer)
{
    agfx_uniform_buffer_object_t ubo = {
        .model = agfx_mat4x4_create_diagonal(1.0f),
        .view = agfx_mat4x4_create_diagonal(1.0f),
        .projection = agfx_mat4x4_create_diagonal(1.0f),
    };

    memcpy(renderer->uniform_buffer_mapped[renderer->state->current_frame], &ubo, sizeof(ubo));
}

agfx_result_t create_descriptor_pool(agfx_renderer_t *renderer)
{
    VkDescriptorPoolSize descriptor_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = AGFX_MAX_FRAMES_IN_FLIGHT
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &descriptor_pool_size,
        .maxSets = AGFX_MAX_FRAMES_IN_FLIGHT
    };

    if (VK_SUCCESS != vkCreateDescriptorPool(renderer->context->device, &descriptor_pool_create_info, NULL, &renderer->descriptor_pool))
    {
        return AGFX_DESCRIPTOR_POOL_ERROR;
    }

    return AGFX_SUCCESS;
}

void free_descriptor_pool(agfx_renderer_t *renderer)
{
    vkDestroyDescriptorPool(renderer->context->device, renderer->descriptor_pool, NULL);
}

agfx_result_t create_descriptor_set(agfx_renderer_t *renderer)
{
    VkDescriptorSetLayout descriptor_set_layouts[AGFX_MAX_FRAMES_IN_FLIGHT] = {
        renderer->descriptor_set_layout,
        renderer->descriptor_set_layout
    };

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = renderer->descriptor_pool,
        .descriptorSetCount = AGFX_MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = descriptor_set_layouts
    };

    renderer->descriptor_sets = malloc(sizeof(VkDescriptorSet) * AGFX_MAX_FRAMES_IN_FLIGHT);

    if (VK_SUCCESS != vkAllocateDescriptorSets(renderer->context->device, &descriptor_set_allocate_info, renderer->descriptor_sets))
    {
        free(renderer->descriptor_sets);
        return AGFX_DESCRIPTOR_SET_ERROR;
    }

    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo descriptor_buffer_info = {
            .buffer = renderer->uniform_buffers[i],
            .offset = 0,
            .range = sizeof(agfx_uniform_buffer_object_t)
        };

        VkWriteDescriptorSet write_descriptor_set = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = renderer->descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &descriptor_buffer_info,
        };

        vkUpdateDescriptorSets(renderer->context->device, 1, &write_descriptor_set, 0, NULL);
    }

    return AGFX_SUCCESS;
}

void free_descriptor_set(agfx_renderer_t *renderer)
{
    free(renderer->descriptor_sets);
}