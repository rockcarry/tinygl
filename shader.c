#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "shader.h"

typedef struct {
    SHADER_COMMON_MEMBERS
    TEXTURE *screen;
    uint32_t color ;
} SHADER;

static void projection_division(float *m)
{
    m[0 * 1 + 0] = m[0 * 1 + 0] / m[3 * 1 + 0];
    m[1 * 1 + 0] = m[1 * 1 + 0] / m[3 * 1 + 0];
    m[2 * 1 + 0] = m[2 * 1 + 0] / m[3 * 1 + 0];
    m[3 * 1 + 0] = 1.0f;
}

static void perspective_vertex(void *ctx, vertex_t *vertex_list, int vertex_num)
{
    SHADER *sd = (SHADER*)ctx;
    int     i;
    for (i = 0; i < vertex_num; i++) {
        float mat_tmp1[4 * 4], mat_tmp2[4 * 4], tempxyz[4 * 1];
        matrix_mul(mat_tmp1, sd->matrix_proj, 4, 4, sd->matrix_view , 4, 4);
        matrix_mul(mat_tmp2, mat_tmp1       , 4, 4, sd->matrix_model, 4, 4);
        matrix_mul(tempxyz , mat_tmp2       , 4, 4, (float*)&vertex_list[i].v, 4, 1);
        projection_division(tempxyz);
        matrix_mul((float*)&vertex_list[i].v, sd->matrix_port, 4, 4, tempxyz, 4, 1);
    }
}

static void wireframe_vertex(void *ctx, vertex_t *vertex_list, int vertex_num)
{
    SHADER *sd = (SHADER*)ctx;
    int     i;
    perspective_vertex(ctx, vertex_list, vertex_num);
    for (i = 0; i < vertex_num; i++) {
        texture_line(sd->screen, vertex_list[i].v.x, vertex_list[i].v.y, vertex_list[(i + 1) % vertex_num].v.x, vertex_list[(i + 1) % vertex_num].v.y, sd->color);
    }
}

static void randcolor_vertex(void *ctx, vertex_t *vertex_list, int vertex_num)
{
    int  i;
    perspective_vertex(ctx, vertex_list, vertex_num);
    for (i = 0; i < vertex_num; i++) {
        vertex_list[i].c.r = (uint8_t)rand();
        vertex_list[i].c.g = (uint8_t)rand();
        vertex_list[i].c.b = (uint8_t)rand();
    }
}

static void fillcolor_fragment(void *ctx, vertex_t *triangle, vec3f_t *bc, vec3i_t *rgb)
{
    *rgb = triangle[0].c;
}

static void interpcolor_fragment(void *ctx, vertex_t *triangle, vec3f_t *bc, vec3i_t *rgb)
{
    rgb->r = bc->alpha * triangle[0].c.r + bc->beta * triangle[1].c.r + bc->gamma * triangle[2].c.r;
    rgb->g = bc->alpha * triangle[0].c.g + bc->beta * triangle[1].c.g + bc->gamma * triangle[2].c.g;
    rgb->b = bc->alpha * triangle[0].c.b + bc->beta * triangle[1].c.b + bc->gamma * triangle[2].c.b;
}

void* shader_init(char *vertex_type, char *fragmt_type)
{
    SHADER *sd = calloc(1, sizeof(SHADER));
    if (!sd) return NULL;
    matrix_identity(sd->matrix_model, 4);
    matrix_identity(sd->matrix_view , 4);
    matrix_identity(sd->matrix_proj , 4);
    matrix_identity(sd->matrix_port , 4);
    sd->color = RGB(0, 255, 0);
    if      (strcmp(vertex_type, "perspective") == 0) sd->vertex   = perspective_vertex;
    else if (strcmp(vertex_type, "wireframe"  ) == 0) sd->vertex   = wireframe_vertex; 
    else if (strcmp(vertex_type, "randcolor"  ) == 0) sd->vertex   = randcolor_vertex;
    if      (strcmp(fragmt_type, "fillcolor"  ) == 0) sd->fragment = fillcolor_fragment;
    else if (strcmp(fragmt_type, "interpcolor") == 0) sd->fragment = interpcolor_fragment;
    return sd;
}

void shader_free(void *ctx)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx) return;
    if (sd->free) sd->free(sd);
    free(sd);
}

void shader_vertex(void *ctx, vertex_t *vertex_list, int vertex_num)
{
    SHADER *sd = (SHADER*)ctx;
    if (sd && sd->vertex) sd->vertex(sd, vertex_list, vertex_num);
}

void shader_fragment(void *ctx, vertex_t *triangle, vec3f_t *bc, vec3i_t *rgb)
{
    SHADER *sd = (SHADER*)ctx;
    if (sd && sd->fragment) sd->fragment(sd, triangle, bc, rgb);
}

void shader_set_matrix(void *ctx, char *type, float *matrix)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx) return;
    if (strcmp(type, "model") == 0) memcpy(sd->matrix_model, matrix, sizeof(sd->matrix_model));
    if (strcmp(type, "view" ) == 0) memcpy(sd->matrix_view , matrix, sizeof(sd->matrix_view ));
    if (strcmp(type, "proj" ) == 0) memcpy(sd->matrix_proj , matrix, sizeof(sd->matrix_proj ));
    if (strcmp(type, "port" ) == 0) memcpy(sd->matrix_port , matrix, sizeof(sd->matrix_port ));
}

void* shader_get_matrix(void *ctx, char *type)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx) return NULL;
    if (strcmp(type, "model") == 0) return sd->matrix_model;
    if (strcmp(type, "view" ) == 0) return sd->matrix_view ;
    if (strcmp(type, "proj" ) == 0) return sd->matrix_proj ;
    if (strcmp(type, "port" ) == 0) return sd->matrix_port ;
    return NULL;
}

void shader_set_param(void *ctx, char *key, void *data)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx) return;
    if (strcmp(key, "screen") == 0) sd->screen = data;
    if (strcmp(key, "color" ) == 0) sd->color  = (uint32_t)data;
}

void shader_get_param(void *ctx, char *key, void *data)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx) return;
    if (strcmp(key, "screen") == 0) *(void   **)data = sd->screen;
    if (strcmp(key, "color" ) == 0) *(uint32_t*)data = sd->color ;
}
