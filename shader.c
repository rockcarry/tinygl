#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shader.h"

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

static int perspective_vertex(struct shader_t *sd, vertex_t t[3])
{
    int  i;
    for (i = 0; i < 3; i++) {
        float mat_tmp1[4 * 4], mat_tmp2[4 * 4], tempxyz[4 * 1];
        matrix_mul(mat_tmp1, sd->matrix_proj, 4, 4, sd->matrix_view , 4, 4);
        matrix_mul(mat_tmp2, mat_tmp1       , 4, 4, sd->matrix_model, 4, 4);
        matrix_mul(tempxyz , mat_tmp2       , 4, 4, (float*)&t[i].v , 4, 1);
        projection_division(tempxyz);
        matrix_mul((float*)&t[i].v, sd->matrix_port, 4, 4, tempxyz, 4, 1);
    }
    return 0;
}

static int wireframe_vertex(struct shader_t *sd, vertex_t t[3])
{
    int  i;
    perspective_vertex(sd, t);
    for (i = 0; i < 3; i++) texture_line(sd->target, t[i].v.x, t[i].v.y, t[(i + 1) % 3].v.x, t[(i + 1) % 3].v.y, sd->color.c);
    return -1;
}

static int randcolor_vertex(struct shader_t *sd, vertex_t t[3])
{
    int  i;
    perspective_vertex(sd, t);
    for (i = 0; i < 3; i++) {
        t[i].c.argb[0] = (uint8_t)rand();
        t[i].c.argb[1] = (uint8_t)rand();
        t[i].c.argb[2] = (uint8_t)rand();
    }
    return 0;
}

static int flatcolor_vertex(struct shader_t *sd, vertex_t t[3])
{
    float intensity;
    int   i;
    vec3f_t v1; vector3f_sub((float*)&v1, (float*)&t[2].v, (float*)&t[0].v);
    vec3f_t v2; vector3f_sub((float*)&v2, (float*)&t[1].v, (float*)&t[0].v);
    vector3f_cross((float*)&t[0].vn, (float*)&v1, (float*)&v2);
    vector3f_norm ((float*)&t[0].vn);
    intensity = vector3f_dot((float*)&t[0].vn, (float*)&sd->light);
    if (intensity < 0) return -1;
    for (i = 0; i < 3; i++) {
        vec4f_t xzyw = t[i].v; matrix_mul((float*)&t[i].v, sd->matrix_port, 4, 4, (void*)&xzyw, 4, 1);
        t[i].c.argb[0] = intensity * sd->color.argb[0];
        t[i].c.argb[1] = intensity * sd->color.argb[1];
        t[i].c.argb[2] = intensity * sd->color.argb[2];
        t[i].c.argb[3] = intensity * sd->color.argb[3];
    }
    return 0;
}

static int gouraud_vertex(struct shader_t *sd, vertex_t t[3])
{
    float   intensity;
    vec4f_t xzyw;
    int     i;
    for (i = 0; i < 3; i++) {
        intensity = vector3f_dot((float*)&t[i].vn, (float*)&sd->light);
        if (intensity < 0) return -1;
        t[i].c.argb[0] = intensity * sd->color.argb[0];
        t[i].c.argb[1] = intensity * sd->color.argb[1];
        t[i].c.argb[2] = intensity * sd->color.argb[2];
        t[i].c.argb[3] = intensity * sd->color.argb[3];
        xzyw = t[i].v; matrix_mul((float*)&t[i].v, sd->matrix_port, 4, 4, (void*)&xzyw, 4, 1);
    }
    return 0;
}

static int fillcolor0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc) { return t[0].c.c; }

static int fillcolor1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    color_t c;
    c.argb[0] = bc->alpha * t[0].c.argb[0] + bc->beta * t[1].c.argb[0] + bc->gamma * t[2].c.argb[0];
    c.argb[1] = bc->alpha * t[0].c.argb[1] + bc->beta * t[1].c.argb[1] + bc->gamma * t[2].c.argb[1];
    c.argb[2] = bc->alpha * t[0].c.argb[2] + bc->beta * t[1].c.argb[2] + bc->gamma * t[2].c.argb[2];
    c.argb[3] = bc->alpha * t[0].c.argb[3] + bc->beta * t[1].c.argb[3] + bc->gamma * t[2].c.argb[3];
    return c.c;
}

static int phongcolor_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    float   intensity;
    vec3f_t vn;
    color_t c;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    intensity = vector3f_dot((float*)&vn, (float*)&sd->light);
    if (intensity < 0) return -1;
    c.argb[0] = intensity * sd->color.argb[0];
    c.argb[1] = intensity * sd->color.argb[1];
    c.argb[2] = intensity * sd->color.argb[2];
    c.argb[3] = intensity * sd->color.argb[3];
    return c.c;
}

static int normal0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    color_t c;
    c.argb[2] = (1.0 - t[0].vn.alpha) * 0.5 * 255;
    c.argb[1] = (1.0 - t[0].vn.beta ) * 0.5 * 255;
    c.argb[0] = (1.0 - t[0].vn.gamma) * 0.5 * 255;
    c.argb[3] = (1.0 - t[0].vn.delta) * 0.5 * 255;
    return c.c;
}

static int normal1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    vec4f_t vn;
    color_t c;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    vn.delta = 0;
    c.argb[2] = (1.0 - vn.alpha) * 0.5 * 255;
    c.argb[1] = (1.0 - vn.beta ) * 0.5 * 255;
    c.argb[0] = (1.0 - vn.gamma) * 0.5 * 255;
    c.argb[3] = (1.0 - vn.delta) * 0.5 * 255;
    return c.c;
}

static int texture0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    float u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    float v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    return texture_getcolor(sd->texture, u * sd->texture->w, v * sd->texture->h);
}

static int texture1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    float   intensity = vector3f_dot((float*)&t[0].vn, (float*)&sd->light);
    float   u, v;
    color_t c;
    if (intensity < 0) return -1;
    u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    c.c = texture_getcolor(sd->texture, u * sd->texture->w, v * sd->texture->h);
    c.argb[0] *= intensity;
    c.argb[1] *= intensity;
    c.argb[2] *= intensity;
    c.argb[3] *= intensity;
    return c.c;
}

static int texture2_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t *bc)
{
    float   intensity, u, v;
    vec3f_t vn;
    color_t c;
    vn.alpha = bc->alpha * t[0].vn.alpha + bc->beta * t[1].vn.alpha + bc->gamma * t[2].vn.alpha;
    vn.beta  = bc->alpha * t[0].vn.beta  + bc->beta * t[1].vn.beta  + bc->gamma * t[2].vn.beta ;
    vn.gamma = bc->alpha * t[0].vn.gamma + bc->beta * t[1].vn.gamma + bc->gamma * t[2].vn.gamma;
    intensity = vector3f_dot((float*)&vn, (float*)&sd->light);
    if (intensity < 0) return -1;
    u = bc->alpha * t[0].vt.u + bc->beta * t[1].vt.u + bc->gamma * t[2].vt.u;
    v = bc->alpha * t[0].vt.v + bc->beta * t[1].vt.v + bc->gamma * t[2].vt.v;
    c.c = texture_getcolor(sd->texture, u * sd->texture->w, v * sd->texture->h);
    c.argb[0] *= intensity;
    c.argb[1] *= intensity;
    c.argb[2] *= intensity;
    c.argb[3] *= intensity;
    return c.c;
}

static struct {
    char *name;      int (*vertex)(struct shader_t *sd, vertex_t t[3]);
} s_vlist[] = {
    { "perspective", perspective_vertex  },
    { "wire"       , wireframe_vertex  },
    { "rand"       , randcolor_vertex  },
    { "flat"       , flatcolor_vertex  },
    { "gouraud"    , gouraud_vertex    },
    { NULL         , NULL              },
};

static struct {
    char *name;      int (*fragmt)(struct shader_t *sd, vertex_t t[3], vec3f_t *bc);
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

static void set_vertex(SHADER *sd, char *name)
{
    for (int i = 0; name && s_vlist[i].name; i++) { if (strcmp(name, s_vlist[i].name) == 0) { sd->vertex = s_vlist[i].vertex; break; } }
}

static void set_fragmt(SHADER *sd, char *name)
{
    for (int i = 0; name && s_flist[i].name; i++) { if (strcmp(name, s_flist[i].name) == 0) { sd->fragmt = s_flist[i].fragmt; break; } }
}

SHADER* shader_init(char *vertex, char *fragmt)
{
    SHADER *sd = calloc(1, sizeof(SHADER));
    if (!sd) return NULL;
    sd->target = sd->texture = sd->deftext = texture_init(0, 0);
    if (!sd->deftext) { free(sd); return NULL; }
    texture_fillrect(sd->deftext, 0, 0, sd->deftext->w, sd->deftext->h, RGB(0, 255, 0));
    sd->color .c = RGB(0, 255, 0);
    sd->light .z =  -1;
    sd->camera.z =   3;
    sd->vertex   = flatcolor_vertex;
    sd->fragmt   = fillcolor0_fragmt;
    matrix_identity  (sd->matrix_model, 4);
    matrix_identity  (sd->matrix_view , 4);
    matrix_identity  (sd->matrix_port , 4);
    matrix_projection(sd->matrix_proj, &sd->camera);
    set_vertex(sd, vertex);
    set_fragmt(sd, fragmt);
    return sd;
}

void shader_free(SHADER *sd) {
    if (sd) {
        texture_free(sd->deftext);
        free(sd);
    }
}

void shader_set_param(SHADER *sd, char *name, void *data)
{
    if (!sd || !name) return;
    if      (strcmp(name, "model"  ) == 0) memcpy(sd->matrix_model, data, sizeof(sd->matrix_model));
    else if (strcmp(name, "view"   ) == 0) memcpy(sd->matrix_view , data, sizeof(sd->matrix_view ));
    else if (strcmp(name, "proj"   ) == 0) memcpy(sd->matrix_proj , data, sizeof(sd->matrix_proj ));
    else if (strcmp(name, "port"   ) == 0) memcpy(sd->matrix_port , data, sizeof(sd->matrix_port ));
    else if (strcmp(name, "target" ) == 0) sd->target = data;
    else if (strcmp(name, "texture") == 0) sd->texture= data;
    else if (strcmp(name, "color"  ) == 0) sd->color  =*(color_t*)data;
    else if (strcmp(name, "light"  ) == 0) sd->light  =*(vec3f_t*)data;
    else if (strcmp(name, "camera" ) == 0) { sd->camera = *(vec3f_t*)data; matrix_projection(sd->matrix_proj, data); }
    else if (strcmp(name, "vertex" ) == 0) set_vertex(sd, (char*)data);
    else if (strcmp(name, "fragmt" ) == 0) set_fragmt(sd, (char*)data);
}

void* shader_get_param(SHADER *sd, char *name)
{
    if (!sd || !name) return NULL;
    if      (strcmp(name, "model"  ) == 0) return sd->matrix_model;
    else if (strcmp(name, "view"   ) == 0) return sd->matrix_view ;
    else if (strcmp(name, "proj"   ) == 0) return sd->matrix_proj ;
    else if (strcmp(name, "port"   ) == 0) return sd->matrix_port ;
    else if (strcmp(name, "target" ) == 0) return sd->target;
    else if (strcmp(name, "texture") == 0) return sd->texture;
    else if (strcmp(name, "color"  ) == 0) return &sd->color;
    else if (strcmp(name, "light"  ) == 0) return &sd->light;
    else if (strcmp(name, "camera" ) == 0) return &sd->camera;
    else if (strcmp(name, "vertex" ) == 0) return sd->vertex;
    else if (strcmp(name, "fragmt" ) == 0) return sd->fragmt;
    return NULL;
}
