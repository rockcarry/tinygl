#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "shader.h"

typedef struct {
    SHADER_COMMON_MEMBERS
    TEXTURE *screen;
    TEXTURE *deftext;
} SHADER;

static void matrix_projection(float *m, vec3f_t *camera)
{
    matrix_identity(m , 4);
    m[3 * 4 + 2] = -1.0 / camera->z;
}

static void projection_division(float *m)
{
    m[0 * 1 + 0] = m[0 * 1 + 0] / m[3 * 1 + 0];
    m[1 * 1 + 0] = m[1 * 1 + 0] / m[3 * 1 + 0];
    m[2 * 1 + 0] = m[2 * 1 + 0] / m[3 * 1 + 0];
    m[3 * 1 + 0] = 1.0f;
}

static int perspective_vertex(struct shader_common_t *sdc, vertex_t t[3])
{
    int  i;
    for (i = 0; i < 3; i++) {
        float mat_tmp1[4 * 4], mat_tmp2[4 * 4], tempxyz[4 * 1];
        matrix_mul(mat_tmp1, sdc->matrix_proj, 4, 4, sdc->matrix_view , 4, 4);
        matrix_mul(mat_tmp2, mat_tmp1        , 4, 4, sdc->matrix_model, 4, 4);
        matrix_mul(tempxyz , mat_tmp2        , 4, 4, (float*)&t[i].v  , 4, 1);
        projection_division(tempxyz);
        matrix_mul((float*)&t[i].v, sdc->matrix_port, 4, 4, tempxyz, 4, 1);
    }
    return 0;
}

static int wireframe_vertex(struct shader_common_t *sdc, vertex_t t[3])
{
    int  i;
    perspective_vertex(sdc, t);
    for (i = 0; i < 3; i++) texture_line(((SHADER*)sdc)->screen, t[i].v.x, t[i].v.y, t[(i + 1) % 3].v.x, t[(i + 1) % 3].v.y, RGB(sdc->color.r, sdc->color.g, sdc->color.b));
    return -1;
}

static int randcolor_vertex(struct shader_common_t *sdc, vertex_t t[3])
{
    int  i;
    perspective_vertex(sdc, t);
    for (i = 0; i < 3; i++) {
        t[i].c.r = (uint8_t)rand();
        t[i].c.g = (uint8_t)rand();
        t[i].c.b = (uint8_t)rand();
    }
    return 0;
}

static int flatcolor_vertex(struct shader_common_t *sdc, vertex_t t[3])
{
    float intensity;
    int   i;
    vec3f_t v1; vector3f_sub((float*)&v1, (float*)&t[2].v, (float*)&t[0].v);
    vec3f_t v2; vector3f_sub((float*)&v2, (float*)&t[1].v, (float*)&t[0].v);
    vector3f_cross((float*)&t[0].vn, (float*)&v1, (float*)&v2);
    vector3f_norm ((float*)&t[0].vn);
    intensity = vector3f_dot((float*)&t[0].vn, (float*)&sdc->light);
    if (intensity < 0) return -1;
    for (i = 0; i < 3; i++) {
        vec4f_t xzyw = t[i].v; matrix_mul((float*)&t[i].v, sdc->matrix_port, 4, 4, (void*)&xzyw, 4, 1);
        t[i].c.r = intensity * sdc->color.r;
        t[i].c.g = intensity * sdc->color.g;
        t[i].c.b = intensity * sdc->color.b;
    }
    return 0;
}

static int gouraud_vertex(struct shader_common_t *sdc, vertex_t t[3])
{
    float   intensity;
    vec4f_t xzyw;
    int     i;
    for (i = 0; i < 3; i++) {
        intensity = vector3f_dot((float*)&t[i].vn, (float*)&sdc->light);
        if (intensity < 0) return -1;
        t[i].c.r = intensity * sdc->color.r;
        t[i].c.g = intensity * sdc->color.g;
        t[i].c.b = intensity * sdc->color.b;
        xzyw = t[i].v; matrix_mul((float*)&t[i].v, sdc->matrix_port, 4, 4, (void*)&xzyw, 4, 1);
    }
    return 0;
}

static int fillcolor0_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb) { *rgb = t[0].c; return 0; }

static int fillcolor1_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    rgb->r = bc->alpha * t[0].c.r + bc->beta * t[1].c.r + bc->gamma * t[2].c.r;
    rgb->g = bc->alpha * t[0].c.g + bc->beta * t[1].c.g + bc->gamma * t[2].c.g;
    rgb->b = bc->alpha * t[0].c.b + bc->beta * t[1].c.b + bc->gamma * t[2].c.b;
    return 0;
}

static int phongcolor_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    float intensity;
    vec3f_t vn;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    intensity = vector3f_dot((float*)&vn, (float*)&sdc->light);
    if (intensity < 0) return -1;
    rgb->r = intensity * sdc->color.r;
    rgb->g = intensity * sdc->color.g;
    rgb->b = intensity * sdc->color.b;
    return 0;
}

static int normal0_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    rgb->r = (1.0 - t[0].vn.alpha) * 0.5 * 255;
    rgb->g = (1.0 - t[0].vn.beta ) * 0.5 * 255;
    rgb->b = (1.0 - t[0].vn.gamma) * 0.5 * 255;
    return 0;
}

static int normal1_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    vec3f_t vn;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    rgb->r = (1.0 - vn.alpha) * 0.5 * 255;
    rgb->g = (1.0 - vn.beta ) * 0.5 * 255;
    rgb->b = (1.0 - vn.gamma) * 0.5 * 255;
    return 0;
}

static int texture0_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    float u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    float v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    texture_getrgb(sdc->texture, u * sdc->texture->width, v * sdc->texture->height, (uint8_t*)&rgb->r, (uint8_t*)&rgb->g, (uint8_t*)&rgb->b);
    return 0;
}

static int texture1_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    float intensity = vector3f_dot((float*)&t[0].vn, (float*)&sdc->light);
    float u, v;
    if (intensity < 0) return -1;
    u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    texture_getrgb(sdc->texture, u * sdc->texture->width, v * sdc->texture->height, (uint8_t*)&rgb->r, (uint8_t*)&rgb->g, (uint8_t*)&rgb->b);
    rgb->r = intensity * (uint8_t)rgb->r;
    rgb->g = intensity * (uint8_t)rgb->g;
    rgb->b = intensity * (uint8_t)rgb->b;
    return 0;
}

static int texture2_fragmt(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    float intensity, u, v;
    vec3f_t vn;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    intensity = vector3f_dot((float*)&vn, (float*)&sdc->light);
    if (intensity < 0) return -1;
    u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    texture_getrgb(sdc->texture, u * sdc->texture->width, v * sdc->texture->height, (uint8_t*)&rgb->r, (uint8_t*)&rgb->g, (uint8_t*)&rgb->b);
    rgb->r = intensity * (uint8_t)rgb->r;
    rgb->g = intensity * (uint8_t)rgb->g;
    rgb->b = intensity * (uint8_t)rgb->b;
    return 0;
}

static struct {
    char *name;      int (*vertex)(struct shader_common_t *sdc, vertex_t t[3]);
} s_vlist[] = {
    { "perspective", perspective_vertex  },
    { "wire"       , wireframe_vertex  },
    { "rand"       , randcolor_vertex  },
    { "flat"       , flatcolor_vertex  },
    { "gouraud"    , gouraud_vertex    },
    { NULL         , NULL              },
};

static struct {
    char *name;      int (*fragmt)(struct shader_common_t *sdc, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb);
} s_flist[] = {
    { "color0"     , fillcolor0_fragmt },
    { "color1"     , fillcolor1_fragmt },
    { "phong"      , phongcolor_fragmt },
    { "normal0"    , normal0_fragmt    },
    { "normal1"    , normal1_fragmt    },
    { "texture0"   , texture0_fragmt   },
    { "texture1"   , texture1_fragmt   },
    { "texture2"   , texture2_fragmt   },
    { NULL         , NULL              },
};

void* shader_init(char *vertex, char *fragmt)
{
    int     i;
    SHADER *sd = calloc(1, sizeof(SHADER));
    if (!sd) return NULL;
    sd->deftext = texture_init(0, 0, 0);
    if (!sd->deftext) { free(sd); return NULL; }
    texture_fillrect(sd->deftext, 0, 0, 8, 8, RGB(0, 255, 0));
    sd->texture  = sd->deftext;
    sd->screen   = sd->deftext;
    sd->color .g = 255;
    sd->light .z =  -1;
    sd->camera.z =   3;
    sd->vertex   = flatcolor_vertex;
    sd->fragmt   = fillcolor0_fragmt;
    matrix_identity  (sd->matrix_model, 4);
    matrix_identity  (sd->matrix_view , 4);
    matrix_identity  (sd->matrix_port , 4);
    matrix_projection(sd->matrix_proj, &sd->camera);
    for (i = 0; vertex && s_vlist[i].name; i++) { if (strcmp(vertex, s_vlist[i].name) == 0) { sd->vertex = s_vlist[i].vertex; break; } }
    for (i = 0; fragmt && s_flist[i].name; i++) { if (strcmp(fragmt, s_flist[i].name) == 0) { sd->fragmt = s_flist[i].fragmt; break; } }
    return sd;
}

void shader_free(void *ctx) {
    SHADER *sd = (SHADER*)ctx;
    if (sd) {
        texture_free(sd->deftext);
        free(sd);
    }
}

int shader_vertex(void *ctx, vertex_t t[3])
{
    SHADER *sd = (SHADER*)ctx;
    return sd->vertex(ctx, t);
}

int shader_fragment(void *ctx, vertex_t t[3], vec3f_t *bc, vec3i_t *rgb)
{
    SHADER *sd = (SHADER*)ctx;
    return sd->fragmt(ctx, t, bc, rgb);
}

void shader_set_param(void *ctx, char *name, void *data)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx || !name) return;
    if      (strcmp(name, "model"  ) == 0) memcpy(sd->matrix_model, data, sizeof(sd->matrix_model));
    else if (strcmp(name, "view"   ) == 0) memcpy(sd->matrix_view , data, sizeof(sd->matrix_view ));
    else if (strcmp(name, "proj"   ) == 0) memcpy(sd->matrix_proj , data, sizeof(sd->matrix_proj ));
    else if (strcmp(name, "port"   ) == 0) memcpy(sd->matrix_port , data, sizeof(sd->matrix_port ));
    else if (strcmp(name, "screen" ) == 0) sd->screen = data;
    else if (strcmp(name, "texture") == 0) sd->texture= data;
    else if (strcmp(name, "color"  ) == 0) sd->color  =*(vec3i_t*)data;
    else if (strcmp(name, "light"  ) == 0) sd->light  =*(vec3f_t*)data;
    else if (strcmp(name, "camera" ) == 0) {
        sd->camera = *(vec3f_t*)data;
        matrix_projection(sd->matrix_proj, data);
    }
}

void* shader_get_param(void *ctx, char *name)
{
    SHADER *sd = (SHADER*)ctx;
    if (!ctx || !name) return NULL;
    if      (strcmp(name, "model"  ) == 0) return sd->matrix_model;
    else if (strcmp(name, "view"   ) == 0) return sd->matrix_view ;
    else if (strcmp(name, "proj"   ) == 0) return sd->matrix_proj ;
    else if (strcmp(name, "port"   ) == 0) return sd->matrix_port ;
    else if (strcmp(name, "screen" ) == 0) return sd->screen;
    else if (strcmp(name, "texture") == 0) return sd->texture;
    else if (strcmp(name, "color"  ) == 0) return &sd->color;
    else if (strcmp(name, "light"  ) == 0) return &sd->light;
    else if (strcmp(name, "camera" ) == 0) return &sd->camera;
    return NULL;
}
