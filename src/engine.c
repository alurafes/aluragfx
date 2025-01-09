#include "engine.h"

agfx_result_t agfx_initialize_engine(agfx_engine_t* engine)
{
    agfx_result_t result = AGFX_SUCCESS;

    SDL_Init(SDL_INIT_EVERYTHING);

    engine->state = (agfx_state_t) {
        .current_frame = 0,
        .quit = 0,
        .resized = 0,
        .rotation = 0
    };

    agfx_create_present(&engine->present);
    agfx_create_context(&engine->present, &engine->context);
    agfx_create_swapchain(&engine->context, &engine->present, &engine->swapchain);
    agfx_create_renderer(&engine->context, &engine->swapchain, &engine->state, &engine->renderer);
    engine->swapchain.renderer = &engine->renderer; // bruh
    agfx_create_framebuffers(&engine->swapchain);
    return result;
}

void agfx_free_engine(agfx_engine_t* engine)
{
    vkDeviceWaitIdle(engine->context.device);

    agfx_free_renderer(&engine->renderer);
    agfx_free_swapchain(&engine->swapchain);
    agfx_free_context(&engine->context);
    agfx_free_present(&engine->present);

    SDL_Quit();
}

void agfx_game_loop(agfx_engine_t* engine)
{
    VkResult result;
    // todo result handling
    vkWaitForFences(engine->context.device, 1, &engine->renderer.in_flight_fences[engine->state.current_frame], VK_TRUE, UINT64_MAX);
    uint32_t image_index;
    result = vkAcquireNextImageKHR(engine->context.device, engine->swapchain.swapchain, UINT64_MAX, engine->renderer.image_available_semaphores[engine->state.current_frame], 0, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        agfx_recreate_swapchain(&engine->swapchain);
        return;
    }

    vkResetFences(engine->context.device, 1, &engine->renderer.in_flight_fences[engine->state.current_frame]);
    vkResetCommandBuffer(engine->renderer.command_buffers[engine->state.current_frame], 0);
    agfx_record_command_buffers(&engine->renderer, image_index);

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    agfx_update_uniform_buffer(&engine->renderer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &engine->renderer.image_available_semaphores[engine->state.current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &engine->renderer.render_finished_semaphores[engine->state.current_frame],
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &engine->renderer.command_buffers[engine->state.current_frame],
    };

    vkQueueSubmit(engine->context.graphics_queue, 1, &submit_info, engine->renderer.in_flight_fences[engine->state.current_frame]);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &engine->renderer.render_finished_semaphores[engine->state.current_frame],
        .swapchainCount = 1,
        .pSwapchains = &engine->swapchain.swapchain,
        .pImageIndices = &image_index
    };

    result = vkQueuePresentKHR(engine->context.present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || engine->state.resized)
    {
        engine->state.resized = 0;
        agfx_recreate_swapchain(&engine->swapchain);
    }

    engine->state.current_frame = (engine->state.current_frame + 1) % AGFX_MAX_FRAMES_IN_FLIGHT;
}

void agfx_main(agfx_engine_t* engine)
{
    while (!engine->state.quit)
    {
        SDL_Event event;
        SDL_WaitEvent(&event);

        switch (event.type)
        {
        case SDL_QUIT:
            engine->state.quit = 1;
            goto event_switch_end;
        
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                engine->state.resized = 1;
            }
            goto event_switch_end;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_LEFT) {
                printf("z = %f\n", engine->state.rotation.z);
                engine->state.rotation.z -= 0.02;
            }
            if (event.key.keysym.sym == SDLK_RIGHT) {
                 printf("z = %f\n", engine->state.rotation.z);
                engine->state.rotation.z += 0.02;
            }
            if (event.key.keysym.sym == SDLK_UP) {
                printf("y = %f\n", engine->state.rotation.y);
                engine->state.rotation.y -= 0.02;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                 printf("y = %f\n", engine->state.rotation.y);
                engine->state.rotation.y += 0.02;
            }
            if (event.key.keysym.sym == SDLK_PAGEUP) {
                printf("x = %f\n", engine->state.rotation.x);
                engine->state.rotation.x -= 0.02;
            }
            if (event.key.keysym.sym == SDLK_PAGEDOWN) {
                 printf("x = %f\n", engine->state.rotation.x);
                engine->state.rotation.x += 0.02;
            }
            goto event_switch_end;
        }
event_switch_end:
        agfx_game_loop(engine);
    }
}