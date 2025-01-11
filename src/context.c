#include "context.h"

agfx_result_t create_vulkan_instance(agfx_context_t* context)
{
    agfx_result_t result = AGFX_SUCCESS;
    
    VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "aluragfx",
        .pEngineName = "aluragfx",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t extensionCount = 0;

    if (SDL_TRUE != SDL_Vulkan_GetInstanceExtensions(context->present->window, &extensionCount, NULL))
    {
        return AGFX_VULKAN_EXTENSIONS_ERROR;
    }
    const char** extensions = (const char**)malloc(sizeof(const char*) * extensionCount);
    if (SDL_TRUE != SDL_Vulkan_GetInstanceExtensions(context->present->window, &extensionCount, extensions))
    {
        result = AGFX_VULKAN_EXTENSIONS_ERROR;
        goto free_extensions;
    }

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
    };

    if (VK_SUCCESS != vkCreateInstance(&instance_create_info, NULL, &context->instance))
    {
        result = AGFX_VULKAN_INSTANCE_ERROR;
        goto free_extensions;
    }

free_extensions:
    free(extensions);
finish:
    return result;
}

agfx_result_t create_vulkan_surface(agfx_context_t* context)
{
    if (SDL_TRUE != SDL_Vulkan_CreateSurface(context->present->window, context->instance, &context->surface))
    {
        return AGFX_VULKAN_SURFACE_ERROR;
    }
    return AGFX_SUCCESS;
}

agfx_result_t find_physical_device(agfx_context_t* context)
{
    uint32_t physical_device_count = 0;
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(context->instance, &physical_device_count, NULL))
    {
        return AGFX_NO_PHYSICAL_DEVICE_ERROR;
    }
    if (physical_device_count == 0)
    {
        return AGFX_NO_PHYSICAL_DEVICE_ERROR;
    }
    VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physical_device_count);
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices))
    {
        free(physical_devices);
        return AGFX_NO_PHYSICAL_DEVICE_ERROR;
    }

    for (uint32_t device_index = 0; device_index < physical_device_count; ++device_index)
    {
        uint32_t physical_device_is_valid = 1;
        uint32_t queue_family_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[device_index], &queue_family_property_count, NULL);
        if (queue_family_property_count == 0)
        {
            physical_device_is_valid = 0;
            goto finish;
        }
        VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_property_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[device_index], &queue_family_property_count, queue_family_properties);

        agfx_queue_family_indices_t queue_family_indices = {
            .graphics_index = -1,
            .present_index = -1
        };

        for (uint32_t i = 0; i < queue_family_property_count; ++i)
        {
            if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queue_family_indices.graphics_index == -1) 
            {
                queue_family_indices.graphics_index = i;
            }

            VkBool32 support;
            if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[device_index], i, context->surface, &support))
            {
                goto queue_family_property_loop_end;
            }
            
            if (VK_TRUE == support && queue_family_indices.present_index == -1)
            {
                queue_family_indices.present_index = i;
            }

            queue_family_property_loop_end:

        }

        if (queue_family_indices.graphics_index == -1 || queue_family_indices.present_index == -1)
        {
            physical_device_is_valid = 0;
            goto free_queue_family_properties;
        }

        uint32_t extension_property_count = 0;
        if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(physical_devices[device_index], NULL, &extension_property_count, NULL))
        {
            physical_device_is_valid = 0;
            goto free_queue_family_properties;
        }
        if (extension_property_count == 0)
        {
            physical_device_is_valid = 0;
            goto free_queue_family_properties;
        }
        VkExtensionProperties* extension_properties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_property_count);
        if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(physical_devices[device_index], NULL, &extension_property_count, extension_properties))
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        }

        uint32_t extension_index = -1;
        for (int i = 0; i < extension_property_count; ++i)
        {
            // todo: check array of strings
            if (strcmp(extension_properties[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                extension_index = i;
            }
        }
        if (extension_index == -1)
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        }

        VkSurfaceCapabilitiesKHR surface_capabilities;
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[device_index], context->surface, &surface_capabilities))
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        }

        uint32_t surface_format_count = 0;
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[device_index], context->surface, &surface_format_count, NULL))
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        } 

        if (surface_format_count == 0)
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        }

        VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * surface_format_count);
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[device_index], context->surface, &surface_format_count, surface_formats))
        {
            physical_device_is_valid = 0;
            goto free_surface_formats;
        } 

        uint32_t present_mode_count = 0;
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[device_index], context->surface, &present_mode_count, NULL))
        {
            physical_device_is_valid = 0;
            goto free_surface_formats;
        }

        if (present_mode_count == 0)
        {
            physical_device_is_valid = 0;
            goto free_surface_formats;
        }

        VkPresentModeKHR* present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * present_mode_count);
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[device_index], context->surface, &present_mode_count, present_modes))
        {
            physical_device_is_valid = 0;
            goto free_present_modes;
        }

    free_present_modes:
        free(present_modes);
    free_surface_formats:
        free(surface_formats);
    free_extension_properties:
        free(extension_properties);
    free_queue_family_properties:
        free(queue_family_properties);
    
    finish:
        if (physical_device_is_valid == 1)
        {
            context->physical_device = physical_devices[device_index];
            context->queue_family_indices = queue_family_indices;
            free(physical_devices);
            return AGFX_SUCCESS;
        }
    }

    free(physical_devices);
    return AGFX_NO_PHYSICAL_DEVICE_ERROR;
}

size_t get_unique_queue_family_indices(agfx_queue_family_indices_t* const queue_family_indices, uint32_t* output_indices_array)
{
    if (AGFX_QUEUE_FAMILY_INDICES_LENGTH == 0) return 0;

    uint32_t indices[AGFX_QUEUE_FAMILY_INDICES_LENGTH];
    for (int offset = 0; offset < AGFX_QUEUE_FAMILY_INDICES_LENGTH; ++offset)
    {
        indices[offset] = *(((uint32_t*)queue_family_indices) + offset);
    }

    qsort(indices, AGFX_QUEUE_FAMILY_INDICES_LENGTH, sizeof(uint32_t), unsigned_integer_compare);

    size_t unique_size = 1;
    output_indices_array[0] = indices[0];

    for (int i = 1; i < AGFX_QUEUE_FAMILY_INDICES_LENGTH; i++) {
        if (indices[i] != indices[i - 1]) 
        {
            output_indices_array[unique_size++] = indices[i];
        }        
    }

    return unique_size;
}

agfx_result_t create_logical_device(agfx_context_t* context)
{
    agfx_result_t result = AGFX_SUCCESS;

    uint32_t unique_indices[AGFX_QUEUE_FAMILY_INDICES_LENGTH];
    size_t unique_indices_size = get_unique_queue_family_indices(&context->queue_family_indices, unique_indices);

    if (unique_indices_size == 0)
    {
        return AGFX_QUEUE_FAMILY_INDICES_ERROR;
    }

    VkDeviceQueueCreateInfo* device_queue_create_infos = (VkDeviceQueueCreateInfo*)calloc(unique_indices_size, sizeof(VkDeviceQueueCreateInfo));
    if (NULL == device_queue_create_infos)
    {
        return AGFX_QUEUE_FAMILY_INDICES_ERROR;
    }

    for (int i = 0; i < unique_indices_size; ++i)
    {
        device_queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_infos[i].queueFamilyIndex = 0;
        device_queue_create_infos[i].pQueuePriorities = &(float) {1.0f};
        device_queue_create_infos[i].queueCount = 1;
    }

    const char* enabledExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkPhysicalDeviceFeatures deviceFeatures = {
        .geometryShader = VK_TRUE,
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = device_queue_create_infos,
        .queueCreateInfoCount = 1,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &enabledExtensionName,
        .pEnabledFeatures = &deviceFeatures
    };

    if (VK_SUCCESS != vkCreateDevice(context->physical_device, &device_create_info, NULL, &context->device))
    {
        result = AGFX_DEVICE_CREATE_ERROR;
    }

    vkGetDeviceQueue(context->device, context->queue_family_indices.graphics_index, 0, &context->graphics_queue);
    vkGetDeviceQueue(context->device, context->queue_family_indices.present_index, 0, &context->present_queue);

    free(device_queue_create_infos);
    return result;
}

agfx_result_t agfx_create_context(agfx_present_t* present, agfx_context_t* out_context)
{
    agfx_context_t context;
    agfx_result_t result;

    context.present = present;

    result = create_vulkan_instance(&context);
    if (AGFX_SUCCESS != result) goto finish;

    result = create_vulkan_surface(&context);
    if (AGFX_SUCCESS != result) goto free_vulkan_instance;

    result = find_physical_device(&context);
    if (AGFX_SUCCESS != result) goto free_vulkan_instance;

    result = create_logical_device(&context);
    if (AGFX_SUCCESS != result) goto free_vulkan_surface;

goto finish;

free_vulkan_surface:
    free_vulkan_surface(&context);
free_vulkan_instance:
    free_vulkan_instance(&context);
finish:
    *out_context = context;
    return result;
}

void agfx_free_context(agfx_context_t* context)
{
    free_logical_device(context);
    free_vulkan_surface(context);
    free_vulkan_instance(context);
}

void free_vulkan_instance(agfx_context_t* context)
{
    vkDestroyInstance(context->instance, NULL);
}

void free_logical_device(agfx_context_t* context)
{
    vkDestroyDevice(context->device, NULL);
}

void free_vulkan_surface(agfx_context_t* context)
{
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
}