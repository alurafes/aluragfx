#ifndef ALURA_GLB_PARSER_TYPES_H
#define ALURA_GLB_PARSER_TYPES_H

#include <stdint.h>

typedef enum agltf_result_t {
    AGLTF_SUCCESS,
    AGLTF_FILE_OPEN_ERROR,
    AGLTF_NOT_GLTF_FILE_ERROR,
    AGLTF_UNSUPPORTED_GLTF_VERSION_ERROR,
    AGLTF_EMPTY_GLTF_FILE_ERROR,
    AGLTF_EMPTY_GLTF_CHUNK_ERROR,
    AGLTF_INVALID_GLTF_CHUNK_TYPE_ERROR,
    AGLTF_INVALID_JSON_STRING_ERROR,
    AGLTF_INVALID_JSON_STRUCTURE_ERROR,
} agltf_result_t;

typedef struct agltf_json_buffer_view_t {
    size_t index;
    uint32_t buffer;
    uint32_t byte_length;
    uint32_t byte_offset;
    uint32_t target;
} agltf_json_buffer_view_t;

typedef enum agltf_json_component_type_t {
    AGLTF_JSON_COMPONENT_TYPE_SIGNED_BYTE = 5120,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_BYTE,
    AGLTF_JSON_COMPONENT_TYPE_SIGNED_SHORT,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_SHORT,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_INT = 5125,
    AGLTF_JSON_COMPONENT_TYPE_FLOAT,
    AGLTF_JSON_COMPONENT_TYPE_UNKNOWN,
} agltf_json_component_type_t;

typedef enum agltf_json_accessor_type_t {
    AGLTF_JSON_ACCESSOR_TYPE_SCALAR,
    AGLTF_JSON_ACCESSOR_TYPE_VEC2,
    AGLTF_JSON_ACCESSOR_TYPE_VEC3,
    AGLTF_JSON_ACCESSOR_TYPE_VEC4,
    AGLTF_JSON_ACCESSOR_TYPE_MAT2,
    AGLTF_JSON_ACCESSOR_TYPE_MAT3,
    AGLTF_JSON_ACCESSOR_TYPE_MAT4,
    AGLTF_JSON_ACCESSOR_TYPE_UNKNOWN
} agltf_json_accessor_type_t;

typedef enum agltf_json_magnigication_filter_t {
    AGLTF_JSON_MAGNIGICATION_FILTER_NEAREST = 9728,
    AGLTF_JSON_MAGNIGICATION_FILTER_LINEAR = 9729,
    AGLTF_JSON_MAGNIGICATION_FILTER_UNKNOWN,
} agltf_json_magnigication_filter_t;

typedef enum agltf_json_minification_filter_t {
    AGLTF_JSON_MINIFICATION_FILTER_NEAREST = 9728,
    AGLTF_JSON_MINIFICATION_FILTER_LINEAR = 9729,
    AGLTF_JSON_MINIFICATION_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
    AGLTF_JSON_MINIFICATION_FILTER_LINEAR_MIPMAP_NEAREST = 9985,
    AGLTF_JSON_MINIFICATION_FILTER_NEAREST_MIPMAP_LINEAR = 9986,
    AGLTF_JSON_MINIFICATION_FILTER_LINEAR_MIPMAP_LINEAR = 9987,
    AGLTF_JSON_MINIFICATION_FILTER_UNKNOWN,
} agltf_json_minification_filter_t;

typedef enum agltf_json_image_mime_type_t {
    AGLTF_JSON_IMAGE_MIME_TYPE_IMAGE_PNG,
    AGLTF_JSON_IMAGE_MIME_TYPE_IMAGE_JPEG,
    AGLTF_JSON_IMAGE_MIME_TYPE_UNKNOWN,
} agltf_json_image_mime_type_t;

typedef struct agltf_accessor_data_t {
    void* data;
    size_t size;
    uint8_t number_of_components;
    uint8_t size_of_element;
} agltf_accessor_data_t;

typedef struct agltf_image_data_t {
    void* data;
    size_t size;
} agltf_image_data_t;

typedef struct agltf_json_accessor_t {
    size_t index;
    agltf_json_buffer_view_t* buffer_view;
    agltf_json_component_type_t component_type;
    uint32_t count;
    agltf_json_accessor_type_t type;
    agltf_accessor_data_t data;
} agltf_json_accessor_t;

typedef struct agltf_json_sampler_t {
    size_t index;
    agltf_json_magnigication_filter_t mag_filter;
    agltf_json_minification_filter_t min_filter;
} agltf_json_sampler_t;

typedef struct agltf_json_image_t {
    size_t index;
    agltf_json_buffer_view_t* buffer_view;
    agltf_json_image_mime_type_t mime_type;
    agltf_image_data_t data;
    // uri images... screw em!!!!!!!!! imagine using external images...
} agltf_json_image_t;

typedef struct agltf_json_texture_t {
    size_t index;
    agltf_json_sampler_t* sampler;
    agltf_json_image_t* source;
} agltf_json_texture_t;

typedef struct agltf_json_texture_info_t {
    agltf_json_texture_t* texture;
    uint32_t tex_coord;
} agltf_json_texture_info_t;

typedef struct agltf_json_material_pbr_t {
    float base_color_factor[4];
    agltf_json_texture_info_t base_color_texture;
    float metallic_factor;
    float roughness_factor;
    // agltf_json_texture_info_t metallic_roughness_texture;
} agltf_json_material_pbr_t;

typedef struct agltf_json_material_t {
    size_t index;
    char* name;
    agltf_json_material_pbr_t pbr;
} agltf_json_material_t;

typedef struct agltf_json_mesh_primitive_attribute_t {
    char* name;
    agltf_json_accessor_t* accessor;
} agltf_json_mesh_primitive_attribute_t;

typedef struct agltf_json_mesh_primitive_t {
    size_t index;
    size_t attribute_count;
    agltf_json_mesh_primitive_attribute_t* attributes;
    agltf_json_accessor_t* indices;
    agltf_json_material_t* material;
    // there are more primitives, but right now I don't care about any other
} agltf_json_mesh_primitive_t;

typedef struct agltf_json_mesh_t {
    char* name;
    size_t primitives_count; 
    agltf_json_mesh_primitive_t* primitives;
} agltf_json_mesh_t;

typedef struct agltf_stat_t {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
} agltf_stat_t;

typedef struct agltf_chunk_t {
    uint32_t chunk_length;
    uint32_t chunk_type;
    char* chunk_data;
} agltf_chunk_t;

typedef struct agltf_glb_t {
    size_t buffer_views_count;
    agltf_json_buffer_view_t* buffer_views;
    size_t accessors_count;
    agltf_json_accessor_t* accessors;
    size_t meshes_count;
    agltf_json_mesh_t* meshes;
    size_t samplers_count;
    agltf_json_sampler_t* samplers;
    size_t images_count;
    agltf_json_image_t* images;
    size_t textures_count;
    agltf_json_texture_t* textures;
    size_t materials_count;
    agltf_json_material_t* materials;
} agltf_glb_t;

#endif