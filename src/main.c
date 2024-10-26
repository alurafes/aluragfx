#include "engine.h"

int main(int argc, char* args[]) 
{
    agfx_engine_t engine;
    agfx_result_t engine_initialize_result = agfx_initialize_engine(&engine);
    if (AGFX_SUCCESS != engine_initialize_result)
    {
        printf("error code: %d\n", engine_initialize_result);
        return engine_initialize_result;
    }
    agfx_main(&engine);
    agfx_free_engine(&engine);

    return 0;
}
