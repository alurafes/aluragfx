#include "aluragltf/include/glb.h"

uint8_t get_accessor_type_number_of_componenets(agltf_json_accessor_type_t type)
{
    switch (type)
    {
        case AGLTF_JSON_ACCESSOR_TYPE_SCALAR: return 1;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC2: return 2;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC3: return 3;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC4: 
        case AGLTF_JSON_ACCESSOR_TYPE_MAT2: return 4;
        case AGLTF_JSON_ACCESSOR_TYPE_MAT3: return 9;
        case AGLTF_JSON_ACCESSOR_TYPE_MAT4: return 16;
        default: return 0;
    }
}

uint8_t get_component_type_element_size(agltf_json_component_type_t type)
{
    switch (type)
    {
        case AGLTF_JSON_COMPONENT_TYPE_SIGNED_BYTE: return sizeof(char);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_BYTE: return sizeof(unsigned char);
        case AGLTF_JSON_COMPONENT_TYPE_SIGNED_SHORT: return sizeof(int16_t);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_SHORT: return sizeof(uint16_t);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_INT: return sizeof(uint32_t);
        case AGLTF_JSON_COMPONENT_TYPE_FLOAT: return sizeof(float);
        default: return 0;
    }
}

agltf_json_accessor_type_t get_accessor_type_from_string(char* accessor_type_string)
{
    if (strcmp(accessor_type_string, "SCALAR") == 0) return AGLTF_JSON_ACCESSOR_TYPE_SCALAR;
    if (strcmp(accessor_type_string, "VEC2") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC2;
    if (strcmp(accessor_type_string, "VEC3") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC3;
    if (strcmp(accessor_type_string, "VEC4") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC4;
    if (strcmp(accessor_type_string, "MAT2") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT2;
    if (strcmp(accessor_type_string, "MAT3") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT3;
    if (strcmp(accessor_type_string, "MAT4") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT4;
    return AGLTF_JSON_ACCESSOR_TYPE_UNKNOWN;
} 

agltf_json_component_type_t get_component_type_from_value(uint32_t component_type_value)
{
    if (component_type_value < 5120 || component_type_value > 5126) return AGLTF_JSON_COMPONENT_TYPE_UNKNOWN;
    return (agltf_json_component_type_t) component_type_value;
}

agltf_result_t read_file_header(FILE* file, agltf_stat_t* stat)
{
    fread(&stat->magic, sizeof(uint32_t), 1, file);
    if (stat->magic != AGLTF_MAGIC)
    {
        return AGLTF_NOT_GLTF_FILE_ERROR;
    }
    fread(&stat->version, sizeof(uint32_t), 1, file);
    if (stat->version != 2)
    {
        return AGLTF_UNSUPPORTED_GLTF_VERSION_ERROR;
    }
    fread(&stat->length, sizeof(uint32_t), 1, file);
    if (stat->length == 0)
    {
        return AGLTF_EMPTY_GLTF_FILE_ERROR;
    }
    return AGLTF_SUCCESS;
}

agltf_result_t read_chunk(FILE* file, agltf_chunk_t* chunk)
{
    fread(&chunk->chunk_length, sizeof(uint32_t), 1, file);
    if (chunk->chunk_length == 0)
    {
        return AGLTF_EMPTY_GLTF_CHUNK_ERROR;
    }
    fread(&chunk->chunk_type, sizeof(uint32_t), 1, file);
    if (chunk->chunk_type != AGLTF_CHUNK_TYPE_JSON && chunk->chunk_type != AGLTF_CHUNK_TYPE_BIN)
    {
        return AGLTF_INVALID_GLTF_CHUNK_TYPE_ERROR;
    }
    chunk->chunk_data = malloc(chunk->chunk_length);
    size_t read_bytes = fread(chunk->chunk_data, sizeof(uint8_t), chunk->chunk_length, file);
    if (read_bytes == 0 || read_bytes != chunk->chunk_length)
    {
        return AGLTF_EMPTY_GLTF_CHUNK_ERROR;
    }
    return AGLTF_SUCCESS;
}

void free_chunk_data(agltf_chunk_t* chunk)
{
    free(chunk->chunk_data);
}

agltf_result_t set_buffer_view_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_buffer_views = cJSON_GetObjectItem(object, "bufferViews");

    if (json_buffer_views == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_buffer_view;
    gltf->buffer_views_count = cJSON_GetArraySize(json_buffer_views);
    gltf->buffer_views = malloc(gltf->buffer_views_count * sizeof(agltf_json_buffer_view_t));
    size_t buffer_view_index = 0;
    cJSON_ArrayForEach(json_buffer_view, json_buffer_views)
    {
        agltf_json_buffer_view_t* buffer_view = &gltf->buffer_views[buffer_view_index];
        buffer_view->index = buffer_view_index;
        buffer_view->buffer = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "buffer"));
        buffer_view->byte_length = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteLength"));
        buffer_view->byte_offset = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteOffset"));
        buffer_view->target = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "target"));
        buffer_view_index++;
    }
    return AGLTF_SUCCESS;
}

void free_buffer_views(agltf_glb_t *gltf)
{
    free(gltf->buffer_views);
}

agltf_result_t set_accessors_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_accessors = cJSON_GetObjectItem(object, "accessors");

    if (json_accessors == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_accessor;
    gltf->accessors_count = cJSON_GetArraySize(json_accessors);
    gltf->accessors = malloc(gltf->accessors_count * sizeof(agltf_json_accessor_t));
    size_t accessor_index = 0;
    cJSON_ArrayForEach(json_accessor, json_accessors)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[accessor_index];
        accessor->index = accessor_index;
        accessor->buffer_view = &gltf->buffer_views[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "bufferView"))];
        accessor->component_type = get_component_type_from_value((uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "componentType")));
        accessor->count = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "count"));
        accessor->type = get_accessor_type_from_string(cJSON_GetStringValue(cJSON_GetObjectItem(json_accessor, "type")));
        accessor_index++;
    }
    return AGLTF_SUCCESS;
}

void free_accessors(agltf_glb_t *gltf)
{
    free(gltf->accessors);
}

agltf_json_magnigication_filter_t get_magnification_filter_from_value(uint32_t magnification_filter_value)
{
    if (magnification_filter_value < 9728 || magnification_filter_value > 9729) return AGLTF_JSON_MAGNIGICATION_FILTER_UNKNOWN;
    return (agltf_json_magnigication_filter_t) magnification_filter_value;
}

agltf_json_minification_filter_t get_minification_filter_from_value(uint32_t minification_filter_value)
{
    if (
        (minification_filter_value < 9728 || minification_filter_value > 9729) &&
        (minification_filter_value < 9984 || minification_filter_value > 9987)
    ) return AGLTF_JSON_MINIFICATION_FILTER_UNKNOWN;
    return (agltf_json_minification_filter_t) minification_filter_value;
}

agltf_result_t set_samplers_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_samplers = cJSON_GetObjectItem(object, "samplers");

    if (json_samplers == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_sampler;
    gltf->samplers_count = cJSON_GetArraySize(json_samplers);
    gltf->samplers = malloc(gltf->samplers_count * sizeof(agltf_json_sampler_t));
    size_t sampler_index = 0;
    cJSON_ArrayForEach(json_sampler, json_samplers)
    {
        agltf_json_sampler_t* sampler = &gltf->samplers[sampler_index];
        sampler->index = sampler_index;
        sampler->mag_filter = get_magnification_filter_from_value((uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_sampler, "magFilter")));
        sampler->min_filter = get_minification_filter_from_value((uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_sampler, "minFilter")));
        sampler_index++;
    }
    return AGLTF_SUCCESS;
}

void free_samplers(agltf_glb_t *gltf)
{
    free(gltf->samplers);
}

agltf_json_image_mime_type_t get_image_mime_type_from_value(char* image_mime_type_value)
{
    if (strcmp(image_mime_type_value, "image/png") == 0) return AGLTF_JSON_IMAGE_MIME_TYPE_IMAGE_PNG;
    if (strcmp(image_mime_type_value, "image/jpg") == 0) return AGLTF_JSON_IMAGE_MIME_TYPE_IMAGE_JPEG;
    if (strcmp(image_mime_type_value, "image/jpeg") == 0) return AGLTF_JSON_IMAGE_MIME_TYPE_IMAGE_JPEG;
    return AGLTF_JSON_IMAGE_MIME_TYPE_UNKNOWN;
}

agltf_result_t set_images_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_images = cJSON_GetObjectItem(object, "images");

    if (json_images == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR; // todo: make images optional lmao
    }

    cJSON* json_image;
    gltf->images_count = cJSON_GetArraySize(json_images);
    gltf->images = malloc(gltf->images_count * sizeof(agltf_json_image_t));
    size_t image_index = 0;
    cJSON_ArrayForEach(json_image, json_images)
    {
        agltf_json_image_t* image = &gltf->images[image_index];
        image->index = image_index;
        image->buffer_view = &gltf->buffer_views[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_image, "bufferView"))];
        image->mime_type = get_image_mime_type_from_value(cJSON_GetStringValue(cJSON_GetObjectItem(json_image, "mimeType")));
        image_index++;
    }
    return AGLTF_SUCCESS;
}

void free_images(agltf_glb_t *gltf)
{
    free(gltf->images);
}

agltf_result_t set_textures_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_textures = cJSON_GetObjectItem(object, "textures");

    if (json_textures == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR; // todo: make images optional lmao
    }

    cJSON* json_texture;
    gltf->textures_count = cJSON_GetArraySize(json_textures);
    gltf->textures = malloc(gltf->textures_count * sizeof(agltf_json_texture_t));
    size_t texture_index = 0;
    cJSON_ArrayForEach(json_texture, json_textures)
    {
        agltf_json_texture_t* texture = &gltf->textures[texture_index];
        texture->index = texture_index;
        texture->sampler = &gltf->samplers[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_texture, "sampler"))];
        texture->source = &gltf->images[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_texture, "source"))];
        texture_index++;
    }
    return AGLTF_SUCCESS;
}

void free_textures(agltf_glb_t *gltf)
{
    free(gltf->textures);
}

agltf_result_t set_materials_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_materials = cJSON_GetObjectItem(object, "materials");

    if (json_materials == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR; // todo: make materials optional
    }

    cJSON* json_material;
    gltf->materials_count = cJSON_GetArraySize(json_materials);
    gltf->materials = calloc(gltf->materials_count, sizeof(agltf_json_material_t));
    size_t material_index = 0;
    cJSON_ArrayForEach(json_material, json_materials)
    {
        agltf_json_material_t* material = &gltf->materials[material_index];
        material->index = material_index;
        material->name = create_from_json_string(json_material, "name");

        cJSON* json_pbr = cJSON_GetObjectItem(json_material, "pbrMetallicRoughness");
        if (json_pbr != NULL)
        {
            material->pbr.metallic_factor = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(json_pbr, "metallicFactor"));
            material->pbr.roughness_factor = (float)cJSON_GetNumberValue(cJSON_GetObjectItem(json_pbr, "roughnessFactor"));

            cJSON* json_base_color_texture = cJSON_GetObjectItem(json_pbr, "baseColorTexture");
            if (json_base_color_texture != NULL)
            {
                material->pbr.base_color_texture.texture = &gltf->textures[(size_t)cJSON_GetNumberValue(cJSON_GetObjectItem(json_base_color_texture, "index"))];
                material->pbr.base_color_texture.tex_coord = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItem(json_base_color_texture, "texCoord"));
            }

            cJSON* json_base_color_factor;
            cJSON* json_base_color_factors = cJSON_GetObjectItem(json_pbr, "baseColorFactor");

            if (json_base_color_factors == NULL)
            {
                material->pbr.base_color_factor[0] = 1.0f;
                material->pbr.base_color_factor[1] = 1.0f;
                material->pbr.base_color_factor[2] = 1.0f;
                material->pbr.base_color_factor[3] = 1.0f;
            } else {
                size_t index = 0;
                cJSON_ArrayForEach(json_base_color_factor, json_base_color_factors)
                {
                    if (index < 4) material->pbr.base_color_factor[index] = (float)cJSON_GetNumberValue(json_base_color_factor);
                    index++;
                }
            }
        }

        material_index++;
    }
    return AGLTF_SUCCESS;
}

void free_materials(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->materials_count; ++i)
    {
        agltf_json_material_t* material = &gltf->materials[i];
        if (material->name != NULL)
        {
            free(material->name);
        }
    }
    free(gltf->materials);
}

agltf_result_t set_primitive_attributes_from_json(agltf_glb_t *gltf, cJSON* object, agltf_json_mesh_primitive_t *primitive)
{
    cJSON* json_attributes = cJSON_GetObjectItem(object, "attributes");

    if (json_attributes == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    size_t json_attributes_count = calculate_cjson_children_count(json_attributes);
    primitive->attribute_count = json_attributes_count;
    primitive->attributes = malloc(json_attributes_count * sizeof(agltf_json_mesh_primitive_attribute_t));
    
    cJSON* json_attributes_child = json_attributes->child;
    size_t json_attributes_index = 0;
    while (json_attributes_child != NULL)
    {
        agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[json_attributes_index];
        attribute->name = create_string_copy(json_attributes_child->string);
        attribute->accessor = &gltf->accessors[(size_t) json_attributes_child->valuedouble];
        json_attributes_child = json_attributes_child->next;
        json_attributes_index++;
    }
    return AGLTF_SUCCESS;
}

void free_primitive_attributes(agltf_json_mesh_primitive_t *primitive)
{
    for (size_t i = 0; i < primitive->attribute_count; ++i)
    {
        agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[i];
        free(attribute->name);
    }
    free(primitive->attributes);
}

agltf_result_t set_primitives_from_json(agltf_glb_t *gltf, cJSON* object, agltf_json_mesh_t *mesh)
{
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON* json_primitives = cJSON_GetObjectItem(object, "primitives");

    if (json_primitives == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_primitive;
    mesh->primitives_count = cJSON_GetArraySize(json_primitives);
    mesh->primitives = malloc(mesh->primitives_count * sizeof(agltf_json_mesh_primitive_t));
    size_t primitive_index = 0;
    cJSON_ArrayForEach(json_primitive, json_primitives)
    {
        agltf_json_mesh_primitive_t* primitive = &mesh->primitives[primitive_index];
        primitive->index = primitive_index;

        result = set_primitive_attributes_from_json(gltf, json_primitive, primitive);
        if (result != AGLTF_SUCCESS)
        {
            free(mesh->primitives);
            return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
        }
    
        if (cJSON_HasObjectItem(json_primitive, "indices"))
        {
            primitive->indices = &gltf->accessors[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_primitive, "indices"))];
        }

        if (cJSON_HasObjectItem(json_primitive, "material"))
        {
            primitive->material = &gltf->materials[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_primitive, "material"))];
        }
        primitive_index++;
    }
    return result;
}

void free_primitives(agltf_json_mesh_t *mesh)
{
    for (size_t i = 0; i < mesh->primitives_count; ++i)
    {
        agltf_json_mesh_primitive_t* primitive = &mesh->primitives[i];
        free_primitive_attributes(primitive);
    }
    free(mesh->primitives);
}

agltf_result_t set_meshses_from_json(cJSON* object, agltf_glb_t *gltf)
{
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON* json_meshes = cJSON_GetObjectItem(object, "meshes");

    if (json_meshes == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_mesh;
    gltf->meshes_count = cJSON_GetArraySize(json_meshes);
    gltf->meshes = malloc(gltf->meshes_count * sizeof(agltf_json_mesh_t));
    size_t mesh_index = 0;
    cJSON_ArrayForEach(json_mesh, json_meshes)
    {
        agltf_json_mesh_t* mesh = &gltf->meshes[mesh_index];
        mesh->name = create_from_json_string(json_mesh, "name"); // mallocs

        result = set_primitives_from_json(gltf, json_mesh, mesh);
        if (result != AGLTF_SUCCESS)
        {
            free(mesh->name);
            free(gltf->meshes);
            return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
        }

        mesh_index++;
    }
    return result;
}

void free_meshes(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->meshes_count; ++i)
    {
        agltf_json_mesh_t* mesh = &gltf->meshes[i];
        free(mesh->name);
        free_primitives(mesh);
    }
    free(gltf->meshes);
}

agltf_result_t parse_gltf_json(char* json_string, size_t json_length, agltf_glb_t *gltf)
{
    printf("%s\n", json_string);
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON *json = cJSON_ParseWithLength(json_string, json_length);
    if (json == NULL)
    {
        return AGLTF_INVALID_JSON_STRING_ERROR;
    }
    result = set_buffer_view_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_json;

    result = set_accessors_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_buffer_views;

    result = set_images_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_accessors;
    
    result = set_samplers_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_images;

    result = set_textures_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_samplers;

    result = set_materials_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_textures;

    result = set_meshses_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_materials;

    cJSON_Delete(json);

goto finish;

free_materials:
    free_materials(gltf);
free_textures:
    free_textures(gltf);
free_samplers:
    free_samplers(gltf);
free_images:
    free_images(gltf);
free_accessors:
    free_accessors(gltf);
free_buffer_views:
    free_buffer_views(gltf);
free_json:
    cJSON_Delete(json);
finish:
    return result;    
}

void free_gltf_json(agltf_glb_t *gltf)
{
    free_meshes(gltf);
    free_materials(gltf);
    free_textures(gltf);
    free_samplers(gltf);
    free_images(gltf);
    free_accessors(gltf);
    free_buffer_views(gltf);
}

agltf_result_t set_accessors_data(agltf_glb_t *gltf, agltf_chunk_t *chunk)
{
    for (size_t i = 0; i < gltf->accessors_count; ++i)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[i];
        accessor->data.number_of_components = get_accessor_type_number_of_componenets(accessor->type);
        accessor->data.size = accessor->count * get_component_type_element_size(accessor->component_type) * get_accessor_type_number_of_componenets(accessor->type);
        accessor->data.data = malloc(accessor->data.size);
        accessor->data.size_of_element = get_component_type_element_size(accessor->component_type);
        memcpy(accessor->data.data, &chunk->chunk_data[accessor->buffer_view->byte_offset], accessor->data.size);
    }
    return AGLTF_SUCCESS;
}

void free_accessors_data(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->accessors_count; ++i)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[i];
        free(accessor->data.data);
    }
}

agltf_result_t set_images_data(agltf_glb_t *gltf, agltf_chunk_t *chunk)
{
    for (size_t i = 0; i < gltf->images_count; ++i)
    {
        agltf_json_image_t* image = &gltf->images[i];

        image->data.size = image->buffer_view->byte_length;
        image->data.data = malloc(image->data.size);
        memcpy(image->data.data, &chunk->chunk_data[image->buffer_view->byte_offset], image->data.size);
    }
    return AGLTF_SUCCESS;
}

void free_images_data(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->images_count; ++i)
    {
        agltf_json_image_t* image = &gltf->images[i];
        free(image->data.data);
    }
}

agltf_result_t agltf_create_glb(const char* path, agltf_glb_t* gltf)
{
    agltf_result_t result = AGLTF_SUCCESS;
    FILE* glb_file = fopen(path, "rb");
    if (glb_file == NULL)
    {
        return AGLTF_FILE_OPEN_ERROR;
    }

    agltf_stat_t stat;
    result = read_file_header(glb_file, &stat);
    if (result != AGLTF_SUCCESS) goto close_file;

    agltf_chunk_t json_chunk;
    result = read_chunk(glb_file, &json_chunk);
    if (result != AGLTF_SUCCESS) goto close_file;

    result = parse_gltf_json(json_chunk.chunk_data, json_chunk.chunk_length, gltf);
    if (result != AGLTF_SUCCESS) goto free_json_chunk;
    
    agltf_chunk_t binary_chunk;
    result = read_chunk(glb_file, &binary_chunk);
    if (result != AGLTF_SUCCESS) goto free_gltf_json;

    result = set_accessors_data(gltf, &binary_chunk);
    if (result != AGLTF_SUCCESS) goto free_binary_chunk;

    result = set_images_data(gltf, &binary_chunk);
    if (result != AGLTF_SUCCESS) goto free_accessors_data;

    free_chunk_data(&binary_chunk);
    free_chunk_data(&json_chunk);
    fclose(glb_file);

goto finish;

free_accessors_data:
    free_accessors_data(gltf);
free_binary_chunk:
    free_chunk_data(&binary_chunk);
free_gltf_json:
    free_gltf_json(gltf);
free_json_chunk:
    free_chunk_data(&json_chunk);
close_file:
    fclose(glb_file);

finish:
    return result;
}

void agltf_free_glb(agltf_glb_t* gltf)
{
    free_images_data(gltf);
    free_accessors_data(gltf);
    free_gltf_json(gltf);
}