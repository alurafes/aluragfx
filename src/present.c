#include "present.h"

#define AGFX_WINDOW_WIDTH 1000
#define AGFX_WINDOW_HEIGHT 900

agfx_result_t agfx_create_present(agfx_present_t* out_present)
{
    agfx_present_t present;
    agfx_result_t result;

    present.width = AGFX_WINDOW_WIDTH;
    present.height = AGFX_WINDOW_HEIGHT;
    
    result = create_window(&present);
    *out_present = present;
    return result;
}

void agfx_free_present(agfx_present_t* present)
{
    free_window(present);
}

static agfx_result_t create_window(agfx_present_t* present)
{
    present->window = SDL_CreateWindow("aluragfx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, present->width, present->height, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (NULL == present->window)
    {
        return AGFX_WINDOW_CREATE_ERROR;
    }
    return AGFX_SUCCESS;
}

void free_window(agfx_present_t* present)
{
    SDL_DestroyWindow(present->window);
}