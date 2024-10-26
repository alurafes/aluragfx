
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

const int MAX_FRAMES_IN_FLIGHT = 2;

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
    AGFX_BUFFER_ERROR,
    AGFX_BUFFER_COPY_ERROR
} agfx_result_t;

typedef struct agfx_queue_family_indices_t {
    uint32_t graphics_index;
    uint32_t present_index;
} agfx_queue_family_indices_t;

#define AGFX_QUEUE_FAMILY_INDICES_LENGTH sizeof(agfx_queue_family_indices_t) / sizeof(uint32_t)

typedef struct agfx_swapchain_info_t {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t surface_formats_count;
    VkSurfaceFormatKHR* surface_formats;
    uint32_t present_modes_count;
    VkPresentModeKHR* present_modes;
} agfx_swapchain_info_t;

typedef struct agfx_engine_t {
    SDL_Window* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    agfx_queue_family_indices_t queue_family_indices;
    agfx_swapchain_info_t swapchain_info;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_images_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    VkPipeline pipeline;
    VkFramebuffer* framebuffers;
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;
    uint32_t current_frame;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
} agfx_engine_t;

typedef struct agfx_vector2_t {
    float x;
    float y;
} agfx_vector2_t;

typedef struct agfx_vector3_t {
    float x;
    float y;
    float z;
} agfx_vector3_t;


typedef struct agfx_vertex_t {
    agfx_vector2_t position;
    agfx_vector3_t color;
} agfx_vertex_t;

const VkVertexInputBindingDescription agfx_vertex_input_binding_description = {
    .binding = 0,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    .stride = sizeof(agfx_vertex_t)
};

#define AGFX_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_SIZE 2

const VkVertexInputAttributeDescription agfx_vertex_input_attribute_description[AGFX_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_SIZE] = {
    (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(agfx_vertex_t, position)
    },
    (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(agfx_vertex_t, color)
    }
};

#define AGFX_VERTEX_ARRAY_SIZE 4
const agfx_vertex_t agfx_vertices[AGFX_VERTEX_ARRAY_SIZE] = {
    {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}
};

uint32_t find_physical_device(agfx_engine_t* engine);
size_t get_unique_queue_family_indices(agfx_queue_family_indices_t* const queue_family_indices, uint32_t* output_indices_array);
agfx_result_t create_logical_device(agfx_engine_t *engine);
agfx_result_t populate_swapchain_info(agfx_engine_t *engine);
agfx_result_t create_swapchain(agfx_engine_t *engine);
agfx_result_t create_swapchain_image_views(agfx_engine_t *engine);
agfx_result_t create_framebuffers(agfx_engine_t *engine);
agfx_result_t create_render_pass(agfx_engine_t *engine);
agfx_result_t create_pipeline(agfx_engine_t *engine);
agfx_result_t create_command_pool(agfx_engine_t *engine);
agfx_result_t create_command_buffers(agfx_engine_t *engine);
agfx_result_t create_sync_objects(agfx_engine_t *engine);
agfx_result_t record_command_buffers(agfx_engine_t* engine, uint32_t image_index);
agfx_result_t recreate_swapchain(agfx_engine_t *engine);
agfx_result_t create_vertex_buffer(agfx_engine_t *engine);

agfx_result_t create_window(agfx_engine_t* engine)
{
    engine->window = SDL_CreateWindow("aluragfx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    if (NULL == engine->window)
    {
        return AGFX_WINDOW_CREATE_ERROR;
    }
    return AGFX_SUCCESS;
}

void free_window(agfx_engine_t* engine)
{
    SDL_DestroyWindow(engine->window);
}

agfx_result_t create_vulkan_instance(agfx_engine_t* engine)
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

    if (SDL_TRUE != SDL_Vulkan_GetInstanceExtensions(engine->window, &extensionCount, NULL))
    {
        return AGFX_VULKAN_EXTENSIONS_ERROR;
    }
    const char** extensions = (const char**)malloc(sizeof(const char*) * extensionCount);
    if (SDL_TRUE != SDL_Vulkan_GetInstanceExtensions(engine->window, &extensionCount, extensions))
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

    if (VK_SUCCESS != vkCreateInstance(&instance_create_info, NULL, &engine->instance))
    {
        result = AGFX_VULKAN_INSTANCE_ERROR;
        goto free_extensions;
    }

free_extensions:
    free(extensions);
finish:
    return result;
}

void free_vulkan_instance(agfx_engine_t* engine)
{
    vkDestroyInstance(engine->instance, NULL);
}

agfx_result_t create_vulkan_surface(agfx_engine_t* engine)
{
    if (SDL_TRUE != SDL_Vulkan_CreateSurface(engine->window, engine->instance, &engine->surface))
    {
        return AGFX_VULKAN_SURFACE_ERROR;
    }
    return AGFX_SUCCESS;
}

void free_vulkan_surface(agfx_engine_t* engine)
{
    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
}

void free_logical_device(agfx_engine_t* engine)
{
    vkDestroyDevice(engine->device, NULL);
}

void free_swapchain_info(agfx_engine_t* engine)
{
    free(engine->swapchain_info.surface_formats);
    free(engine->swapchain_info.present_modes);
}

void free_swapchain(agfx_engine_t* engine)
{
    free(engine->swapchain_images);
    vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
}

void free_swapchain_image_views(agfx_engine_t* engine)
{
    for (int i = 0; i < engine->swapchain_images_count; ++i)
    {
        vkDestroyImageView(engine->device, engine->swapchain_image_views[i], NULL);
    }
    free(engine->swapchain_image_views);
}

void free_pipeline(agfx_engine_t* engine) {
    vkDestroyPipeline(engine->device, engine->pipeline, NULL);
    vkDestroyPipelineLayout(engine->device, engine->pipeline_layout, NULL);
}

void free_render_pass(agfx_engine_t* engine)
{
    vkDestroyRenderPass(engine->device, engine->render_pass, NULL);
}

void free_framebuffers(agfx_engine_t* engine)
{
    for (int i = 0; i < engine->swapchain_images_count; ++i)
    {
        vkDestroyFramebuffer(engine->device, engine->framebuffers[i], NULL);
    }
    free(engine->framebuffers);
}

void free_command_pool(agfx_engine_t* engine)
{
    vkDestroyCommandPool(engine->device, engine->command_pool, NULL);
}

void free_command_buffers(agfx_engine_t* engine)
{
    vkFreeCommandBuffers(engine->device, engine->command_pool, MAX_FRAMES_IN_FLIGHT, engine->command_buffers);
}

void free_sync_objects(agfx_engine_t* engine)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(engine->device, engine->image_available_semaphores[i], NULL);
        vkDestroySemaphore(engine->device, engine->render_finished_semaphores[i], NULL);
        vkDestroyFence(engine->device, engine->in_flight_fences[i], NULL);
    }
}

void free_vertex_buffer(agfx_engine_t *engine)
{
    vkDestroyBuffer(engine->device, engine->vertex_buffer, NULL);
    vkFreeMemory(engine->device, engine->vertex_buffer_memory, NULL);
}

agfx_result_t agfx_initialize_engine(agfx_engine_t* pEngine)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    agfx_engine_t engine;

    engine.current_frame = 0;

    agfx_result_t result;

    result = create_window(&engine);
    if (AGFX_SUCCESS != result) goto finish;

    result = create_vulkan_instance(&engine);
    if (AGFX_SUCCESS != result) goto free_window;

    result = create_vulkan_surface(&engine);
    if (AGFX_SUCCESS != result) goto free_vulkan_instance;

    result = find_physical_device(&engine);
    if (AGFX_SUCCESS != result) goto free_vulkan_surface;
    
    result = create_logical_device(&engine);
    if (AGFX_SUCCESS != result) goto free_vulkan_surface;

    result = populate_swapchain_info(&engine);
    if (AGFX_SUCCESS != result) goto free_logical_device;

    result = create_swapchain(&engine);
    if (AGFX_SUCCESS != result) goto free_swapchain_info;

    result = create_swapchain_image_views(&engine);
    if (AGFX_SUCCESS != result) goto free_swapchain;

    result = create_render_pass(&engine);
    if (AGFX_SUCCESS != result) goto free_swapchain_image_views;

    result = create_pipeline(&engine);
    if (AGFX_SUCCESS != result) goto free_render_pass;

    result = create_framebuffers(&engine);
    if (AGFX_SUCCESS != result) goto free_pipeline;

    result = create_command_pool(&engine);
    if (AGFX_SUCCESS != result) goto free_framebuffers;

    result - create_vertex_buffer(&engine);
    if (AGFX_SUCCESS != result) goto free_command_pool;

    result = create_command_buffers(&engine);
    if (AGFX_SUCCESS != result) goto free_vertex_buffer;

    result = create_sync_objects(&engine);
    if (AGFX_SUCCESS != result) goto free_command_buffers;

    goto finish;

free_sync_objects:
    free_sync_objects(&engine);
free_command_buffers:
    free_command_buffers(&engine);
free_vertex_buffer:
    free_vertex_buffer(&engine);
free_command_pool:
    free_command_pool(&engine);
free_framebuffers:
    free_framebuffers(&engine);
free_render_pass:
    free_render_pass(&engine);
free_pipeline:
    free_pipeline(&engine);
free_swapchain_image_views:
    free_swapchain_image_views(&engine);
free_swapchain:
    free_swapchain(&engine);
free_swapchain_info:
    free_swapchain_info(&engine);
free_logical_device:
    free_logical_device(&engine);
free_vulkan_surface:
    free_vulkan_surface(&engine);
free_vulkan_instance:
    free_vulkan_instance(&engine);
free_window:
    free_window(&engine);
finish:
    *pEngine = engine;
    return result;
}

void agfx_free_engine(agfx_engine_t* engine)
{
    free_sync_objects(engine);
    free_command_buffers(engine);
    free_vertex_buffer(engine);
    free_command_pool(engine);
    free_framebuffers(engine);
    free_render_pass(engine);
    free_pipeline(engine);
    free_swapchain_image_views(engine);
    free_swapchain(engine);
    free_swapchain_info(engine);
    free_logical_device(engine);
    free_vulkan_surface(engine);
    free_vulkan_instance(engine);
    free_window(engine);
    SDL_Quit();
}

int main(int argc, char* args[]) 
{
    agfx_engine_t engine;
    agfx_result_t engine_initialize_result = agfx_initialize_engine(&engine);
    if (AGFX_SUCCESS != engine_initialize_result)
    {
        printf("error code: %d\n", engine_initialize_result);
        return engine_initialize_result;
    }

    printf("the game should be running now...\n");

    uint32_t quit = 0;
    uint32_t resized = 0;
    while (!quit)
    {
        SDL_Event event;
        VkResult result;
        SDL_WaitEvent(&event);

        switch (event.type)
        {
        case SDL_QUIT:
            quit = 1;
            goto event_switch_end;
        
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                resized = 1;
            }
            goto event_switch_end;
        }

event_switch_end:


        // drawing

        // todo result handling
        vkWaitForFences(engine.device, 1, &engine.in_flight_fences[engine.current_frame], VK_TRUE, UINT64_MAX);

        uint32_t image_index;
        result = vkAcquireNextImageKHR(engine.device, engine.swapchain, UINT64_MAX, engine.image_available_semaphores[engine.current_frame], VK_NULL_HANDLE, &image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swapchain(&engine);
            continue;
        }

        vkResetFences(engine.device, 1, &engine.in_flight_fences[engine.current_frame]);

        vkResetCommandBuffer(engine.command_buffers[engine.current_frame], 0);
        record_command_buffers(&engine, image_index);

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info = {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &engine.image_available_semaphores[engine.current_frame],
          .signalSemaphoreCount = 1,
          .pSignalSemaphores = &engine.render_finished_semaphores[engine.current_frame],
          .pWaitDstStageMask = wait_stages,
          .commandBufferCount = 1,
          .pCommandBuffers = &engine.command_buffers[engine.current_frame],
        };

        vkQueueSubmit(engine.graphics_queue, 1, &submit_info, engine.in_flight_fences[engine.current_frame]);

        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &engine.render_finished_semaphores[engine.current_frame],
            .swapchainCount = 1,
            .pSwapchains = &engine.swapchain,
            .pImageIndices = &image_index
        };

        result = vkQueuePresentKHR(engine.present_queue, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized)
        {
            resized = 0;
            recreate_swapchain(&engine);
        }

        engine.current_frame = (engine.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vkDeviceWaitIdle(engine.device);

    agfx_free_engine(&engine);
    return 0;
}

agfx_result_t find_physical_device(agfx_engine_t* engine)
{
    uint32_t physical_device_count = 0;
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, NULL))
    {
        return AGFX_NO_PHYSICAL_DEVICE_ERROR;
    }
    if (physical_device_count == 0)
    {
        return AGFX_NO_PHYSICAL_DEVICE_ERROR;
    }
    VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physical_device_count);
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, physical_devices))
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
            if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[device_index], i, engine->surface, &support))
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
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[device_index], engine->surface, &surface_capabilities))
        {
            physical_device_is_valid = 0;
            goto free_extension_properties;
        }

        uint32_t surface_format_count = 0;
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[device_index], engine->surface, &surface_format_count, NULL))
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
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices[device_index], engine->surface, &surface_format_count, surface_formats))
        {
            physical_device_is_valid = 0;
            goto free_surface_formats;
        } 

        uint32_t present_mode_count = 0;
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[device_index], engine->surface, &present_mode_count, NULL))
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
        if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_devices[device_index], engine->surface, &present_mode_count, present_modes))
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
            engine->physical_device = physical_devices[device_index];
            engine->queue_family_indices = queue_family_indices;
            free(physical_devices);
            return AGFX_SUCCESS;
        }
    }

    free(physical_devices);
    return AGFX_NO_PHYSICAL_DEVICE_ERROR;
}

int unsigned_integer_compare(const void *a, const void *b) {
    int value_a = *(uint32_t *)a;
    int value_b = *(uint32_t *)b;
    return value_a - value_b;
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

agfx_result_t create_logical_device(agfx_engine_t *engine)
{
    agfx_result_t result = AGFX_SUCCESS;

    uint32_t unique_indices[AGFX_QUEUE_FAMILY_INDICES_LENGTH];
    size_t unique_indices_size = get_unique_queue_family_indices(&engine->queue_family_indices, unique_indices);

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
        .geometryShader = VK_TRUE
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = device_queue_create_infos,
        .queueCreateInfoCount = 1,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &enabledExtensionName,
        .pEnabledFeatures = &deviceFeatures
    };

    if (VK_SUCCESS != vkCreateDevice(engine->physical_device, &device_create_info, NULL, &engine->device))
    {
        result = AGFX_DEVICE_CREATE_ERROR;
    }

    vkGetDeviceQueue(engine->device, engine->queue_family_indices.graphics_index, 0, &engine->graphics_queue);
    vkGetDeviceQueue(engine->device, engine->queue_family_indices.present_index, 0, &engine->present_queue);

    free(device_queue_create_infos);
    return result;
}

agfx_result_t populate_swapchain_info(agfx_engine_t *engine)
{
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physical_device, engine->surface, &engine->swapchain_info.surface_capabilities))
    {
        return AGFX_SURFACE_CAPABILITIES_ERROR;
    }
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physical_device, engine->surface, &engine->swapchain_info.surface_formats_count, NULL))
    {
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    if (engine->swapchain_info.surface_formats_count == 0)
    {
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    engine->swapchain_info.surface_formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * engine->swapchain_info.surface_formats_count);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physical_device, engine->surface, &engine->swapchain_info.surface_formats_count, engine->swapchain_info.surface_formats))
    {
        free(engine->swapchain_info.surface_formats);
        return AGFX_SURFACE_FORMATS_ERROR;
    }
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physical_device, engine->surface, &engine->swapchain_info.present_modes_count, NULL))
    {
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    if (engine->swapchain_info.present_modes_count == 0)
    {
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    engine->swapchain_info.present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * engine->swapchain_info.present_modes_count);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physical_device, engine->surface, &engine->swapchain_info.present_modes_count, engine->swapchain_info.present_modes))
    {
        free(engine->swapchain_info.surface_formats);
        free(engine->swapchain_info.present_modes);
        return AGFX_SURFACE_PRESENT_MODES_ERROR;
    }
    
    return AGFX_SUCCESS;
}

agfx_result_t create_swapchain(agfx_engine_t *engine)
{
    VkSurfaceFormatKHR surface_format = engine->swapchain_info.surface_formats[0];
    for (int i = 0; i < engine->swapchain_info.surface_formats_count; ++i)
    {
        if (engine->swapchain_info.surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && engine->swapchain_info.surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = engine->swapchain_info.surface_formats[i];
            break;
        }
    }
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (int i = 0; i < engine->swapchain_info.present_modes_count; ++i)
    {
        if (engine->swapchain_info.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = engine->swapchain_info.present_modes[i];
            break;
        }
    }
    VkExtent2D extent = engine->swapchain_info.surface_capabilities.currentExtent;
    if (engine->swapchain_info.surface_capabilities.currentExtent.width == UINT32_MAX)
    {
        int width, height;
        SDL_Vulkan_GetDrawableSize(engine->window, &width, &height);
        extent.width = fmin(engine->swapchain_info.surface_capabilities.maxImageExtent.width, fmax(width, engine->swapchain_info.surface_capabilities.minImageExtent.width));
        extent.height = fmin(engine->swapchain_info.surface_capabilities.maxImageExtent.height, fmax(height, engine->swapchain_info.surface_capabilities.minImageExtent.height));
    }
    uint32_t image_count = engine->swapchain_info.surface_capabilities.minImageCount + 1;
    if (engine->swapchain_info.surface_capabilities.maxImageCount != 0 && image_count > engine->swapchain_info.surface_capabilities.maxImageCount)
    {
        image_count = engine->swapchain_info.surface_capabilities.maxImageCount;
    }

    uint32_t queue_family_indices_count = 0;
    uint32_t queue_family_indices[2] = {engine->queue_family_indices.graphics_index, engine->queue_family_indices.present_index};

    VkSharingMode sharing_mode = engine->queue_family_indices.graphics_index != engine->queue_family_indices.present_index ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    if (sharing_mode == VK_SHARING_MODE_CONCURRENT)
    {
        queue_family_indices_count = 2;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = engine->surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .pQueueFamilyIndices = queue_family_indices,
        .queueFamilyIndexCount = queue_family_indices_count,
        .preTransform = engine->swapchain_info.surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (VK_SUCCESS != vkCreateSwapchainKHR(engine->device, &swapchain_create_info, NULL, &engine->swapchain))
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &engine->swapchain_images_count, NULL))
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    if (engine->swapchain_images_count == 0)
    {
        return AGFX_SWAPCHAIN_ERROR;
    }
    engine->swapchain_images = (VkImage*)malloc(sizeof(VkImage) * engine->swapchain_images_count);
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &engine->swapchain_images_count, engine->swapchain_images))
    {
        free(engine->swapchain_images);
        return AGFX_SWAPCHAIN_ERROR;
    }
    engine->swapchain_format = surface_format.format;
    engine->swapchain_extent = extent;

    return AGFX_SUCCESS;
}

agfx_result_t create_swapchain_image_views(agfx_engine_t *engine)
{
    engine->swapchain_image_views = (VkImageView*)malloc(sizeof(VkImageView) * engine->swapchain_images_count);
    for (int i = 0; i < engine->swapchain_images_count; ++i)
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
            .format = engine->swapchain_format,
            .components = components,
            .subresourceRange =  subresource_range,
            .image = engine->swapchain_images[i],
        };
        if (VK_SUCCESS != vkCreateImageView(engine->device, &image_view_create_info, NULL, &engine->swapchain_image_views[i]))
        {
            free(engine->swapchain_image_views);
            return AGFX_SWAPCHAIN_IMAGE_VIEW_ERROR;
        }
    }
    return AGFX_SUCCESS;
}

agfx_result_t create_render_pass(agfx_engine_t *engine)
{
    VkAttachmentDescription attachment_description = {
        .format = engine->swapchain_format,
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
        .srcAccessMask = VK_NULL_HANDLE,
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

    if (VK_SUCCESS != vkCreateRenderPass(engine->device, &render_pass_create_info, NULL, &engine->render_pass))
    {
        return AGFX_RENDER_PASS_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t create_framebuffers(agfx_engine_t *engine)
{
    engine->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * engine->swapchain_images_count);
    for (int i = 0; i < engine->swapchain_images_count; ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = engine->render_pass,
            .width = engine->swapchain_extent.width,
            .height = engine->swapchain_extent.height,
            .layers = 1,
            .attachmentCount = 1,
            .pAttachments = &engine->swapchain_image_views[i]
        };
        if (VK_SUCCESS != vkCreateFramebuffer(engine->device, &framebuffer_create_info, NULL, &engine->framebuffers[i]))
        {
            free(engine->framebuffers);
            return AGFX_FRAMEBUFFER_ERROR;
        }
    }
    return AGFX_SUCCESS;
}

agfx_result_t create_command_pool(agfx_engine_t *engine)
{
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = engine->queue_family_indices.graphics_index,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };

    if (VK_SUCCESS != vkCreateCommandPool(engine->device, &command_pool_create_info, NULL, &engine->command_pool))
    {
        return AGFX_COMMAND_POOL_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t create_command_buffers(agfx_engine_t *engine)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = engine->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    engine->command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    if (VK_SUCCESS != vkAllocateCommandBuffers(engine->device, &command_buffer_allocate_info, engine->command_buffers))
    {
        free(engine->command_buffers);
        return AGFX_COMMAND_BUFFERS_ERROR;
    }

    return AGFX_SUCCESS;
}

agfx_result_t record_command_buffers(agfx_engine_t* engine, uint32_t image_index)
{
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info = {};
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pInheritanceInfo = &command_buffer_inheritance_info,
        .flags = VK_NULL_HANDLE,
    };

    if (VK_SUCCESS != vkBeginCommandBuffer(engine->command_buffers[engine->current_frame], &command_buffer_begin_info))
    {
        return AGFX_COMMAND_BUFFERS_ERROR;
    }

    VkRect2D render_area = {
        .offset = {},
        .extent = engine->swapchain_extent,
    };

    VkClearValue clear_value = {
        .color = {
            .float32 = {0.0, 0.0, 0.0, 1.0}
        }
    };

    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = engine->render_pass,
        .framebuffer = engine->framebuffers[image_index],
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    vkCmdBeginRenderPass(engine->command_buffers[engine->current_frame], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(engine->command_buffers[engine->current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, engine->pipeline);
    vkCmdBindVertexBuffers(engine->command_buffers[engine->current_frame], 0, 1, &engine->vertex_buffer, &(VkDeviceSize){0});
    vkCmdDraw(engine->command_buffers[engine->current_frame], AGFX_VERTEX_ARRAY_SIZE * sizeof(agfx_vertices[0]), 1, 0, 0);
    vkCmdEndRenderPass(engine->command_buffers[engine->current_frame]);
    vkEndCommandBuffer(engine->command_buffers[engine->current_frame]);
    
    return AGFX_SUCCESS;
}

agfx_result_t create_pipeline(agfx_engine_t *engine)
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
    if (VK_SUCCESS != vkCreateShaderModule(engine->device, &vert_shader_module_create_info, NULL, &vert_shader_module))
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
    if (VK_SUCCESS != vkCreateShaderModule(engine->device, &frag_shader_module_create_info, NULL, &frag_shader_module))
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
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)engine->swapchain_extent.width,
        .height = (float)engine->swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .extent = engine->swapchain_extent,
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
    };

    if (VK_SUCCESS != vkCreatePipelineLayout(engine->device, &pipeline_layout_create_info, NULL, &engine->pipeline_layout))
    {
        vkDestroyShaderModule(engine->device, vert_shader_module, NULL);
        vkDestroyShaderModule(engine->device, frag_shader_module, NULL);
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
        .layout = engine->pipeline_layout,
        .renderPass = engine->render_pass,
        .subpass = 0,
        .pViewportState = &viewport_state_create_info,
    };

    if (VK_SUCCESS != vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &engine->pipeline))
    {
        vkDestroyShaderModule(engine->device, vert_shader_module, NULL);
        vkDestroyShaderModule(engine->device, frag_shader_module, NULL);
        free(vertex_shader_data);
        free(fragment_shader_data);
        return AGFX_PIPELINE_ERROR;
    }

    vkDestroyShaderModule(engine->device, vert_shader_module, NULL);
    vkDestroyShaderModule(engine->device, frag_shader_module, NULL);

    return AGFX_SUCCESS;
}

agfx_result_t create_sync_objects(agfx_engine_t *engine)
{
    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    engine->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    engine->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    engine->in_flight_fences = (VkFence*)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (VK_SUCCESS != vkCreateSemaphore(engine->device, &semaphore_create_info, NULL, &engine->image_available_semaphores[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }

        if (VK_SUCCESS != vkCreateSemaphore(engine->device, &semaphore_create_info, NULL, &engine->render_finished_semaphores[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }

        if (VK_SUCCESS != vkCreateFence(engine->device, &fence_create_info, NULL, &engine->in_flight_fences[i]))
        {
            return AGFX_SYNC_OBJECT_ERROR;
        }
    }

    return AGFX_SUCCESS;
}

agfx_result_t recreate_swapchain(agfx_engine_t *engine)
{
    vkDeviceWaitIdle(engine->device);
    
    free_framebuffers(engine);
    free_swapchain_image_views(engine);
    free_swapchain(engine);

    agfx_result_t result = AGFX_SUCCESS;

    result = create_swapchain(engine);
    if (AGFX_SUCCESS != result) return result;

    result = create_swapchain_image_views(engine);
    if (AGFX_SUCCESS != result) {
        free_swapchain(engine);
        return result;
    }

    result = create_framebuffers(engine);
    if (AGFX_SUCCESS != result) {
        free_swapchain_image_views(engine);
        free_swapchain(engine);
    }

    return result;
}

uint32_t find_vulkan_memory_type(agfx_engine_t *engine, uint32_t type_filter, VkMemoryPropertyFlags property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(engine->physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }
}

agfx_result_t create_buffer(agfx_engine_t *engine, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (VK_SUCCESS != vkCreateBuffer(engine->device, &buffer_create_info, NULL, buffer))
    {
        return AGFX_BUFFER_ERROR;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(engine->device, *buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = find_vulkan_memory_type(engine, memory_requirements.memoryTypeBits, property_flags),
        .allocationSize = memory_requirements.size
    };

    if (VK_SUCCESS != vkAllocateMemory(engine->device, &memory_allocate_info, NULL, buffer_memory))
    {
        vkDestroyBuffer(engine->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }

    if (VK_SUCCESS != vkBindBufferMemory(engine->device, *buffer, *buffer_memory, 0))
    {
        vkFreeMemory(engine->device, *buffer_memory, NULL);
        vkDestroyBuffer(engine->device, *buffer, NULL);
        return AGFX_BUFFER_ERROR;
    }
}

agfx_result_t copy_buffer(agfx_engine_t *engine, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = engine->command_pool,
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;
    if (VK_SUCCESS != vkAllocateCommandBuffers(engine->device, &command_buffer_allocate_info, &command_buffer))
    {
        return AGFX_BUFFER_COPY_ERROR;
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (VK_SUCCESS != vkBeginCommandBuffer(command_buffer, &begin_info))
    {
        vkFreeCommandBuffers(engine->device, engine->command_pool, 1, &command_buffer);
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

    if (VK_SUCCESS != vkQueueSubmit(engine->graphics_queue, 1, &submit_info, VK_NULL_HANDLE))
    {
        vkFreeCommandBuffers(engine->device, engine->command_pool, 1, &command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    if (VK_SUCCESS != vkQueueWaitIdle(engine->graphics_queue))
    {
        vkFreeCommandBuffers(engine->device, engine->command_pool, 1, &command_buffer);
        return AGFX_BUFFER_COPY_ERROR;
    }

    vkFreeCommandBuffers(engine->device, engine->command_pool, 1, &command_buffer);
    return AGFX_SUCCESS;
}

agfx_result_t create_vertex_buffer(agfx_engine_t *engine)
{
    agfx_result_t result;
    size_t vertex_buffer_size = sizeof(agfx_vertex_t) * AGFX_VERTEX_ARRAY_SIZE;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    result = create_buffer(engine, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        return result;
    }

    void* buffer;
    if (VK_SUCCESS != vkMapMemory(engine->device, staging_buffer_memory, 0, vertex_buffer_size, 0, &buffer))
    {
        vkFreeMemory(engine->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(engine->device, staging_buffer, NULL);
        return AGFX_VERTEX_BUFFER_ERROR;
    }
    memcpy(buffer, agfx_vertices, vertex_buffer_size);
    vkUnmapMemory(engine->device, staging_buffer_memory);

    result = create_buffer(engine, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->vertex_buffer, &engine->vertex_buffer_memory);
    if (AGFX_SUCCESS != result)
    {
        vkFreeMemory(engine->device, staging_buffer_memory, NULL);
        vkDestroyBuffer(engine->device, staging_buffer, NULL);
        return result;
    }

    copy_buffer(engine, staging_buffer, engine->vertex_buffer, vertex_buffer_size);

    vkFreeMemory(engine->device, staging_buffer_memory, NULL);
    vkDestroyBuffer(engine->device, staging_buffer, NULL);

    return AGFX_SUCCESS;
}