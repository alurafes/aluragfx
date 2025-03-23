#ifndef ALURA_GLTF_UTILS_H
#define ALURA_GLTF_UTILS_H

#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

char* create_string_copy(char* string);
char* create_from_json_string(cJSON* object, char* key);
size_t calculate_cjson_children_count(cJSON* object);

#endif