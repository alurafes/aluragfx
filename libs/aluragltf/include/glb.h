#ifndef ALURA_GLB_PARSER_H
#define ALURA_GLB_PARSER_H

#include <stdio.h>

#include "glb_types.h"
#include "utils.h"

#define AGLTF_MAGIC 0x46546C67
#define AGLTF_CHUNK_TYPE_JSON 0x4E4F534A
#define AGLTF_CHUNK_TYPE_BIN 0x004E4942

agltf_result_t agltf_create_glb(const char* path, agltf_glb_t* result);
void agltf_free_glb(agltf_glb_t* gltf);

#endif