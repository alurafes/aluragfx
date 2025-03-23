#include "aluragltf/include/utils.h"

char* create_string_copy(char* string)
{
    if (string == NULL) return NULL;
    char* destination = (char*)malloc(strlen(string) + 1);
    strcpy(destination, string);
    return destination;
}

char* create_from_json_string(cJSON* object, char* key)
{
    char* string = cJSON_GetStringValue(cJSON_GetObjectItem(object, key));
    return create_string_copy(string);
}

size_t calculate_cjson_children_count(cJSON* object)
{
    size_t count = 0;

    cJSON* child = object->child;
    while (child != NULL)
    {
        count++;
        child = child->next;
    }

    return count;
}