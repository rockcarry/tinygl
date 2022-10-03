#ifndef _SHADER_H_
#define _SHADER_H_

#include "matrix.h"
#include "texture.h"

struct shader_common_t;

#define SHADER_COMMON_MEMBERS \
    float    matrix_model[4 * 4]; \
    float    matrix_view [4 * 4]; \
    float    matrix_proj [4 * 4]; \
    float    matrix_port [4 * 4]; \
    vec3f_t  light, camera;       \
    TEXTURE *texture;             \
    color_t  color ;              \
    int (*vertex)(struct shader_common_t *sdc, vertex_t t[3]); \
    int (*fragmt)(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc);

typedef struct shader_common_t {
    SHADER_COMMON_MEMBERS
} SHADER_COMMON;

void* shader_init(char *vertex, char *fragmt);
void  shader_free(void *ctx);

int   shader_vertex  (void *ctx, vertex_t t[3]);
int   shader_fragment(void *ctx, vertex_t t[3], vec3f_t *bc);

void  shader_set_param(void *ctx, char *name, void *data);
void* shader_get_param(void *ctx, char *name);

#endif
