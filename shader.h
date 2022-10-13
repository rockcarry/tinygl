#ifndef _SHADER_H_
#define _SHADER_H_

#include "matrix.h"
#include "texture.h"

typedef struct shader_t {
    mat4f_t  mat_model, mat_view, mat_proj;
    vec3f_t  light;
    TEXTURE *target;
    TEXTURE *texture;
    TEXTURE *deftext;
    color_t  color;
    int (*vertex)(struct shader_t *sdc, vertex_t t[3]);
    int (*fragmt)(struct shader_t *sdc, vertex_t t[3], vec3f_t bc);
} SHADER;

SHADER* shader_init(char *vertex, char *fragmt);
void    shader_free(SHADER *sd);

void  shader_set(SHADER *sd, char *name, void *data);
void* shader_get(SHADER *sd, char *name);

#endif
