#ifndef _SHADER_H_
#define _SHADER_H_

#include "matrix.h"

#define SHADER_COMMON_MEMBERS    \
    float matrix_model[4 * 4];   \
    float matrix_view [4 * 4];   \
    float matrix_proj [4 * 4];   \
    float matrix_port [4 * 4];   \
    void (*free    )(void *ctx); \
    void (*vertex  )(void *ctx, vertex_t *vertex_list, int vertex_num); \
    void (*fragment)(void *ctx, vertex_t *triangle, vec3f_t *bc, vec3i_t *rgb);

typedef struct {
    SHADER_COMMON_MEMBERS
} SHADER_COMMON;

void* shader_init(char *vertex_type, char *fragmt_type);
void  shader_free(void *ctx );

void  shader_vertex  (void *ctx, vertex_t *vertex_list, int vertex_num);
void  shader_fragment(void *ctx, vertex_t *triangle, vec3f_t *bc, vec3i_t *rgb);

void  shader_set_matrix(void *ctx, char *type, float *matrix);
void* shader_get_matrix(void *ctx, char *type);

void  shader_set_param(void *ctx, char *key, void *data);
void  shader_get_param(void *ctx, char *key, void *data);

#endif
