#include "renderer.h"

#include "math/martix.h"

agfx_result_t create_render_pass(agfx_renderer_t *renderer)
{
    VkAttachmentDescription swapchain_attachment_description = {
        .format = renderer->swapchain->swapchain_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference swapchain_attachment_description_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depth_attachment_description = {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depth_attachment_description_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass_description = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = &swapchain_attachment_description_ref,
        .colorAttachmentCount = 1,
        .pDepthStencilAttachment = &depth_attachment_description_ref
    };

    VkSubpassDependency subpass_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription attachment_descriptions[AGFX_ATTACHMENT_ARRAY_SIZE] = {
        swapchain_attachment_description,
        depth_attachment_description
    };

    VkRenderPassCreateInfo render_pass_create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = AGFX_ATTACHMENT_ARRAY_SIZE,
        .pAttachments = attachment_descriptions,
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


    VkClearValue clear_values[AGFX_ATTACHMENT_ARRAY_SIZE] = {
        {
            .color = {
                .float32 = {0.5f, 0.5f, 0.5f, 1.0f}
            }
        },
        {
            .depthStencil = {
                .depth = 1.0f,
                .stencil = 0.0f
            }
        }
    };

    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderer->render_pass,
        .framebuffer = renderer->swapchain->framebuffers[image_index],
        .renderArea = render_area,
        .clearValueCount = AGFX_ATTACHMENT_ARRAY_SIZE,
        .pClearValues = clear_values
    };

    vkCmdBeginRenderPass(renderer->command_buffers[renderer->state->current_frame], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)renderer->swapchain->swapchain_extent.width,
        .height = (float)renderer->swapchain->swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    uint32_t viewport_count = 1;
    
    VkRect2D scissor = {
        .extent = renderer->swapchain->swapchain_extent,
        .offset = {0, 0}
    };

    uint32_t scissor_count = 1;

    vkCmdSetViewportWithCount(renderer->command_buffers[renderer->state->current_frame], viewport_count, &viewport);
    vkCmdSetScissorWithCount(renderer->command_buffers[renderer->state->current_frame], scissor_count, &scissor);
    vkCmdBindPipeline(renderer->command_buffers[renderer->state->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline);
    
    for (size_t i = 0; i < renderer->meshes_count; ++i)
    {
        agfx_mesh_t* mesh = &renderer->meshes[i];
        vkCmdBindDescriptorSets(renderer->command_buffers[renderer->state->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout, 0, 1, &mesh->descriptor_sets[renderer->state->current_frame], 0, NULL);
        vkCmdBindVertexBuffers(renderer->command_buffers[renderer->state->current_frame], 0, 1, &mesh->vertex_buffer, &(VkDeviceSize){0});
        vkCmdBindIndexBuffer(renderer->command_buffers[renderer->state->current_frame], mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(renderer->command_buffers[renderer->state->current_frame], mesh->indices_count, 1, 0, 0, 0);
    }


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

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL, // FUN
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
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

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .viewportCount = 0,
        .pViewports = NULL,
        .scissorCount = 0, 
        .pScissors = NULL,
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

    #define AGFX_DYNAMIC_STATE_COUNT 2 
    VkDynamicState dynamic_state[AGFX_DYNAMIC_STATE_COUNT] = {
        VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
    };

    VkPipelineDynamicStateCreateInfo pipeline_dynamic_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = AGFX_DYNAMIC_STATE_COUNT,
        .pDynamicStates = dynamic_state,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

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
        .pDynamicState = &pipeline_dynamic_create_info,
        .pViewportState = &viewport_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
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

agfx_result_t create_vertex_buffer_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    agfx_result_t result;
    size_t vertex_buffer_size = sizeof(mesh->vertices[0]) * mesh->vertices_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    result = agfx_helper_create_buffer(renderer->context, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
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
    memcpy(buffer, mesh->vertices, vertex_buffer_size);
    vkUnmapMemory(renderer->context->device, staging_buffer_memory);

    result = agfx_helper_create_buffer(renderer->context, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->vertex_buffer, &mesh->vertex_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return result;
    }

    result = agfx_helper_copy_buffer(renderer, staging_buffer, mesh->vertex_buffer, vertex_buffer_size);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->vertex_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, mesh->vertex_buffer, NULL);
        return result;
    }

    vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
    vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_index_buffer_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    agfx_result_t result;
    size_t index_buffer_size = sizeof(mesh->indices[0]) * mesh->indices_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    result = agfx_helper_create_buffer(renderer->context, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
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
    memcpy(buffer, mesh->indices, index_buffer_size);
    vkUnmapMemory(renderer->context->device, staging_buffer_memory);

    result = agfx_helper_create_buffer(renderer->context, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->index_buffer, &mesh->index_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return result;
    }

    result = agfx_helper_copy_buffer(renderer, staging_buffer, mesh->index_buffer, index_buffer_size);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->index_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, mesh->index_buffer, NULL);
        return result;
    }

    vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
    vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_descriptor_set_layout(agfx_renderer_t *renderer)
{
    VkDescriptorSetLayoutBinding bindings[AGFX_DESCRIPTOR_COUNT] = {
        {
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .binding = 0,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        },
        {
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .binding = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = NULL,
        }
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = AGFX_DESCRIPTOR_COUNT,
        .pBindings = bindings
    };

    if (VK_SUCCESS != vkCreateDescriptorSetLayout(renderer->context->device, &layout_create_info, NULL, &renderer->descriptor_set_layout))
    {
        return AGFX_DESCRIPTOR_SET_LAYOUT_ERROR;
    }

    return AGFX_SUCCESS;
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

void free_vertex_buffer_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    vkDestroyBuffer(renderer->context->device, mesh->vertex_buffer, NULL);
    vkFreeMemory(renderer->context->device, mesh->vertex_buffer_memory, NULL);
}

void free_index_buffer_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    vkDestroyBuffer(renderer->context->device, mesh->index_buffer, NULL);
    vkFreeMemory(renderer->context->device, mesh->index_buffer_memory, NULL);
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

    result = load_model(&renderer);
    if (AGFX_SUCCESS != result) goto free_command_pool;

    result = create_descriptor_pool(&renderer);
    if (AGFX_SUCCESS != result) goto free_model;

    result = create_descriptor_sets(&renderer);
    if (AGFX_SUCCESS != result) goto free_descriptor_pool;

    result = create_command_buffers(&renderer);
    if (AGFX_SUCCESS != result) goto free_descriptor_set;

    result = create_sync_objects(&renderer);
    if (AGFX_SUCCESS != result) goto free_command_buffers;

goto finish;

// free_sync_objects:
//     free_sync_objects(&renderer);
free_command_buffers:
    free_command_buffers(&renderer);
free_descriptor_set:
    free_descriptor_sets(&renderer);
free_descriptor_pool:
    free_descriptor_pool(&renderer);
free_command_pool:
    free_command_pool(&renderer);
free_pipeline:
    free_pipeline(&renderer);
free_render_pass:
    free_render_pass(&renderer);
free_descriptor_set_layout:
    free_descriptor_set_layout(&renderer);
free_model:
    free_model(&renderer);
finish:
    *out_renderer = renderer;
    return result;
}

void agfx_free_renderer(agfx_renderer_t *renderer)
{
    free_sync_objects(renderer);
    free_command_buffers(renderer);
    free_descriptor_sets(renderer);
    free_descriptor_pool(renderer);
    free_command_pool(renderer);
    free_render_pass(renderer);
    free_pipeline(renderer);
    free_descriptor_set_layout(renderer);
    free_model(renderer);
}

// todo: refactor
// todo: malloc result checks
agfx_result_t create_uniform_buffers_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    agfx_result_t result = AGFX_SUCCESS;

    mesh->uniform_buffers = malloc(sizeof(VkBuffer) * AGFX_MAX_FRAMES_IN_FLIGHT);
    mesh->uniform_buffer_memories = malloc(sizeof(VkDeviceMemory) * AGFX_MAX_FRAMES_IN_FLIGHT);
    mesh->uniform_buffer_mapped = malloc(sizeof(void*) * AGFX_MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        result = agfx_helper_create_buffer(
            renderer->context, 
            sizeof(agfx_uniform_buffer_object_t), 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &mesh->uniform_buffers[i], 
            &mesh->uniform_buffer_memories[i]
        );

        if (AGFX_SUCCESS != result)
        {
            goto free;
        }

        if (VK_SUCCESS != vkMapMemory(renderer->context->device, mesh->uniform_buffer_memories[i], 0, sizeof(agfx_uniform_buffer_object_t), 0, &mesh->uniform_buffer_mapped[i]))
        {
            result = AGFX_BUFFER_MAP_ERROR;
        }

        if (result == AGFX_SUCCESS) goto finish;

        free:
        if (i > 0) {
            for (size_t j = 0; j < i; ++j)
            {
                vkDestroyBuffer(renderer->context->device, mesh->uniform_buffers[j], NULL);
                vkFreeMemory(renderer->context->device, mesh->uniform_buffer_memories[j], NULL);
            }
        }
        free(mesh->uniform_buffers);
        free(mesh->uniform_buffer_memories);
        free(mesh->uniform_buffer_mapped);
        return result;

        finish:
    }

    return result;
}

void free_uniform_buffers_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyBuffer(renderer->context->device, mesh->uniform_buffers[i], NULL);
        vkFreeMemory(renderer->context->device, mesh->uniform_buffer_memories[i], NULL);
    }

    free(mesh->uniform_buffers);
    free(mesh->uniform_buffer_memories);
    free(mesh->uniform_buffer_mapped);
}

void agfx_update_uniform_buffer(agfx_renderer_t *renderer)
{
    for (size_t mesh_index = 0; mesh_index < renderer->meshes_count; ++mesh_index)
    {
        agfx_mesh_t* mesh = &renderer->meshes[mesh_index];
        agfx_uniform_buffer_object_t ubo = {
            .model = agfx_mat4x4_multiplied_by_mat4x4(agfx_mat4x4_multiplied_by_mat4x4(agfx_mat4x4_translation((agfx_vector3_t) {.x = 0.0f, .y = 0.0f, .z = 0.0f}), agfx_mat4x4_rotation_euler(renderer->state->rotation)), agfx_mat4x4_scale((agfx_vector3_t) {.x = 1.0f, .y = 1.0f, .z = 1.0f})),
            .view = agfx_mat4x4_look_at((agfx_vector3_t) {.x = 2.0f, .y = 2.0f, .z = 3.0f}, (agfx_vector3_t) {.x = 0.0f, .y = 0.0f, .z = 1.9f}, (agfx_vector3_t) {.x = 0.0f, .y = 0.0f, .z = 1.0f}),
            .projection = agfx_mat4x4_perspective(renderer->state->camera_fov * M_PI / 180.f, renderer->swapchain->swapchain_extent.width / (float) renderer->swapchain->swapchain_extent.height, 0.1f, 10.0f)
        };

        memcpy(mesh->uniform_buffer_mapped[renderer->state->current_frame], &ubo, sizeof(ubo));
    }

}

agfx_result_t create_descriptor_pool(agfx_renderer_t *renderer)
{
    VkDescriptorPoolSize descriptor_pool_sizes[AGFX_DESCRIPTOR_COUNT] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = AGFX_MAX_FRAMES_IN_FLIGHT * renderer->meshes_count
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = AGFX_MAX_FRAMES_IN_FLIGHT * renderer->meshes_count // so far 1 mesh = 1 texture. In the future imma do something about it. Right now I am desperate to see a bunch of textured cubes and pyramids ;-;
        },
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = AGFX_DESCRIPTOR_COUNT,
        .pPoolSizes = descriptor_pool_sizes,
        .maxSets = AGFX_MAX_FRAMES_IN_FLIGHT * renderer->meshes_count 
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

agfx_result_t create_descriptor_sets(agfx_renderer_t *renderer)
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

    for (size_t mesh_index = 0; mesh_index < renderer->meshes_count; ++mesh_index)
    {
        agfx_mesh_t* mesh = &renderer->meshes[mesh_index];

        mesh->descriptor_sets = malloc(sizeof(VkDescriptorSet) * AGFX_MAX_FRAMES_IN_FLIGHT);

        if (VK_SUCCESS != vkAllocateDescriptorSets(renderer->context->device, &descriptor_set_allocate_info, mesh->descriptor_sets))
        {
            free(mesh->descriptor_sets);
            return AGFX_DESCRIPTOR_SET_ERROR;
        }

        for (size_t i = 0; i < AGFX_MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo descriptor_buffer_info = {
                .buffer = mesh->uniform_buffers[i],
                .offset = 0,
                .range = sizeof(agfx_uniform_buffer_object_t)
            };

            VkDescriptorImageInfo descriptor_image_info = {
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = mesh->texture_image_view,
                .sampler = mesh->texture_sampler,
            };

            VkWriteDescriptorSet write_descriptor_sets[AGFX_DESCRIPTOR_COUNT] = {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mesh->descriptor_sets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .pBufferInfo = &descriptor_buffer_info,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = mesh->descriptor_sets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .pImageInfo = &descriptor_image_info,
                }
            };
            vkUpdateDescriptorSets(renderer->context->device, AGFX_DESCRIPTOR_COUNT, write_descriptor_sets, 0, NULL);
        }
    }

    return AGFX_SUCCESS;
}

void free_descriptor_sets(agfx_renderer_t *renderer)
{
    for (size_t mesh_index = 0; mesh_index < renderer->meshes_count; ++mesh_index)
    {
        agfx_mesh_t* mesh = &renderer->meshes[mesh_index];
        free(mesh->descriptor_sets);
    }
}

// this file is becoming a mess, gotta refactor the crap out of this soon ._.
// but so far i am kinda following the vulkan-tutorial, so imma think about this later

agfx_result_t create_texture_image_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh, void* raw_image_data, size_t raw_image_data_size)
{
    agfx_result_t result = AGFX_SUCCESS;
    SDL_Surface* original_image_surface = IMG_Load_RW(SDL_RWFromConstMem(raw_image_data, raw_image_data_size), 1);
    if (NULL == original_image_surface)
    {
        result = AGFX_IMAGE_LOAD_ERROR;
        return result;
    }

    SDL_Surface* image_surface = SDL_ConvertSurfaceFormat(original_image_surface, SDL_PIXELFORMAT_ABGR8888 , 0);
    SDL_FreeSurface(original_image_surface);
    if (NULL == image_surface)
    {
        result = AGFX_IMAGE_LOAD_ERROR;
        return result;
    }

    VkDeviceSize image_size = image_surface->w * image_surface->h * image_surface->format->BytesPerPixel;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    result = agfx_helper_create_buffer(renderer->context, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        SDL_FreeSurface(image_surface);
        return result;
    }
    void* data;
    result = vkMapMemory(renderer->context->device, staging_buffer_memory, 0, image_size, 0, &data);
    if (AGFX_SUCCESS != result)
    {
        SDL_FreeSurface(image_surface);
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        return result;
    }
    memcpy(data, image_surface->pixels, image_size);
    vkUnmapMemory(renderer->context->device, staging_buffer_memory);

    result = agfx_helper_create_image(renderer->context, image_surface->w, image_surface->h, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->texture_image, &mesh->texture_image_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->texture_image_memory, NULL);
        return result;
    }

    result = agfx_helper_transition_image_layout(renderer, mesh->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->texture_image_memory, NULL);
        return result;
    }

    result = agfx_helper_copy_buffer_to_image(renderer, staging_buffer, mesh->texture_image, image_surface->w, image_surface->h);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->texture_image_memory, NULL);
        return result;
    }

    result = agfx_helper_transition_image_layout(renderer, mesh->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
        vkFreeMemory(renderer->context->device, mesh->texture_image_memory, NULL);
        return result;
    }
    
    SDL_FreeSurface(image_surface);
    vkDestroyBuffer(renderer->context->device, staging_buffer, NULL);
    vkFreeMemory(renderer->context->device, staging_buffer_memory, NULL);
    return result;
}

void free_texture_image_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    vkDestroyImage(renderer->context->device, mesh->texture_image, NULL);
    vkFreeMemory(renderer->context->device, mesh->texture_image_memory, NULL);
}

agfx_result_t create_texture_image_view_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    return agfx_helper_create_image_view(renderer->context, mesh->texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &mesh->texture_image_view);
}

void free_texture_image_view_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    vkDestroyImageView(renderer->context->device, mesh->texture_image_view, NULL);
}

agfx_result_t create_texture_sampler_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(renderer->context->physical_device, &device_properties);
    VkSamplerCreateInfo sampler_create_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = device_properties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };

    if (VK_SUCCESS != vkCreateSampler(renderer->context->device, &sampler_create_info, NULL, &mesh->texture_sampler))
    {
        return AGFX_SAMPLER_CREATE_ERROR;
    }

    return AGFX_SUCCESS;
}

void free_texture_sampler_for_mesh(agfx_renderer_t *renderer, agfx_mesh_t* mesh)
{
    vkDestroySampler(renderer->context->device, mesh->texture_sampler, NULL);
}

agfx_result_t load_model(agfx_renderer_t *renderer)
{
    agfx_result_t result = AGFX_SUCCESS;

    agltf_glb_t model;

    agltf_result_t model_result = agltf_create_glb("./models/test.glb", &model);
    if (model_result != AGLTF_SUCCESS) return AGFX_MODEL_LOAD_ERROR;

    renderer->meshes_count = 0;

    for (size_t mesh_index = 0; mesh_index < model.meshes_count; ++mesh_index)
    {
        renderer->meshes_count += model.meshes[mesh_index].primitives_count;
    }

    renderer->meshes = calloc(renderer->meshes_count, sizeof(agfx_mesh_t));

    size_t engine_mesh_index = 0;
    for (size_t mesh_index = 0; mesh_index < model.meshes_count; ++mesh_index)
    {
        agltf_json_mesh_t* mesh = &model.meshes[mesh_index];
        for (size_t primitive_index = 0; primitive_index < mesh->primitives_count; ++primitive_index)
        {
            agfx_mesh_t* engine_mesh = &renderer->meshes[engine_mesh_index];
            agltf_json_mesh_primitive_t* primitive = &mesh->primitives[primitive_index];
            
            engine_mesh->indices_count = primitive->indices->count;
            engine_mesh->indices = calloc(engine_mesh->indices_count, sizeof(uint32_t));

            void* starting_point = primitive->indices->data.data;
            void* ending_point = starting_point + primitive->indices->data.size;
            for (void* current = starting_point; current != ending_point; current += primitive->indices->data.size_of_element)
            {
                size_t index = (current - starting_point) / primitive->indices->data.size_of_element;
                switch (primitive->indices->component_type)
                {
                    case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_BYTE: engine_mesh->indices[index] = *(uint8_t*)current; goto after_component_type_switch;
                    case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_SHORT: engine_mesh->indices[index] = *(uint16_t*)current; goto after_component_type_switch;
                    case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_INT: engine_mesh->indices[index] = *(uint32_t*)current; goto after_component_type_switch;
                }
                after_component_type_switch:
            }

            for (size_t attribute_index = 0; attribute_index < primitive->attribute_count; ++attribute_index)
            {
                agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[attribute_index];
                if (strcmp("POSITION", attribute->name) == 0)
                {
                    if (attribute->accessor->data.number_of_components != 3)
                    {
                        return AGFX_MODEL_LOAD_ERROR;
                    }
                    engine_mesh->vertices_count = attribute->accessor->count;
                    engine_mesh->vertices = calloc(engine_mesh->vertices_count, sizeof(agfx_vertex_t));

                    float* starting_point = (float*)attribute->accessor->data.data;
                    float* ending_point = starting_point + (attribute->accessor->data.size / sizeof(float));
                    for (float* current = starting_point; current != ending_point; current += attribute->accessor->data.number_of_components)
                    {
                        int vertex_index = (current - starting_point) / attribute->accessor->data.number_of_components;
                        engine_mesh->vertices[vertex_index].position.x = *current;
                        engine_mesh->vertices[vertex_index].position.y = *(current + 1);
                        engine_mesh->vertices[vertex_index].position.z = *(current + 2);
                    }
                }
                if (strcmp("TEXCOORD_0", attribute->name) == 0)
                {
                    if (attribute->accessor->data.number_of_components != 2)
                    {
                        return AGFX_MODEL_LOAD_ERROR;
                    }
                    float* starting_point = (float*)attribute->accessor->data.data;
                    float* ending_point = starting_point + (attribute->accessor->data.size / sizeof(float));
                    for (float* current = starting_point; current != ending_point; current += attribute->accessor->data.number_of_components)
                    {
                        int vertex_index = (current - starting_point) / attribute->accessor->data.number_of_components;
                        engine_mesh->vertices[vertex_index].texture_coordinate.x = *current;
                        engine_mesh->vertices[vertex_index].texture_coordinate.y = *(current + 1);
                    }
                }
            }

            create_index_buffer_for_mesh(renderer, engine_mesh);
            create_vertex_buffer_for_mesh(renderer, engine_mesh);
            create_texture_image_for_mesh(renderer, engine_mesh, mesh->primitives->material->pbr.base_color_texture.texture->source->data.data, mesh->primitives->material->pbr.base_color_texture.texture->source->data.size);
            create_texture_image_view_for_mesh(renderer, engine_mesh);
            create_texture_sampler_for_mesh(renderer, engine_mesh);
            create_uniform_buffers_for_mesh(renderer, engine_mesh);
            engine_mesh_index++;
        }
    }

    agltf_free_glb(&model);

    return result;
}

void free_model(agfx_renderer_t *renderer)
{
    for (size_t i = 0; i < renderer->meshes_count; ++i)
    {
        free_index_buffer_for_mesh(renderer, &renderer->meshes[i]);
        free_vertex_buffer_for_mesh(renderer, &renderer->meshes[i]);
        free_texture_image_for_mesh(renderer, &renderer->meshes[i]);
        free_texture_image_view_for_mesh(renderer, &renderer->meshes[i]);
        free_texture_sampler_for_mesh(renderer, &renderer->meshes[i]);
        free_uniform_buffers_for_mesh(renderer, &renderer->meshes[i]);
    }
}