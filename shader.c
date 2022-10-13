#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shader.h"

static vec3f_t perspective_correction(vertex_t t[3], vec3f_t bc)
{
    vec3f_t v = {{ bc.alpha / t[0].w, bc.beta / t[1].w, bc.gamma / t[2].w }};
    float   f = 1 / ( v.alpha + v.beta + v.gamma );
    return vec3f_mul(v, f);
}

static color_t color_interpolate(vertex_t t[3], vec3f_t bc)
{
    color_t c;
    c.argb[0] = bc.alpha * t[0].c.argb[0] + bc.beta * t[1].c.argb[0] + bc.gamma * t[2].c.argb[0];
    c.argb[1] = bc.alpha * t[0].c.argb[1] + bc.beta * t[1].c.argb[1] + bc.gamma * t[2].c.argb[1];
    c.argb[2] = bc.alpha * t[0].c.argb[2] + bc.beta * t[1].c.argb[2] + bc.gamma * t[2].c.argb[2];
    c.argb[3] = 0;
    return c;
}

static vec4f_t normal_interpolate(vertex_t t[3], vec3f_t bc)
{
    vec4f_t vn;
    vn.alpha = bc.alpha * t[0].vn.alpha + bc.beta * t[1].vn.alpha + bc.gamma * t[2].vn.alpha;
    vn.beta  = bc.alpha * t[0].vn.beta  + bc.beta * t[1].vn.beta  + bc.gamma * t[2].vn.beta ;
    vn.gamma = bc.alpha * t[0].vn.gamma + bc.beta * t[1].vn.gamma + bc.gamma * t[2].vn.gamma;
    vn.delta = 1;
    return vn;
}

static vec2f_t uv_interpolate(vertex_t t[3], vec3f_t bc)
{
    vec2f_t uv = {{ bc.alpha * t[0].vt.u + bc.beta * t[1].vt.u + bc.gamma * t[2].vt.u, bc.alpha * t[0].vt.v + bc.beta * t[1].vt.v + bc.gamma * t[2].vt.v }};
    return uv;
}

static color_t color_from_normal(vec4f_t vn)
{
    return color_rgb((1.0 - vn.alpha) * 0.5 * 255, (1.0 - vn.beta ) * 0.5 * 255, (1.0 - vn.gamma) * 0.5 * 255);
}

static color_t color_intensity(color_t c, float intensity)
{
    float fb = intensity * c.argb[0]; if (fb > 255) fb = 255;
    float fg = intensity * c.argb[1]; if (fg > 255) fg = 255;
    float fr = intensity * c.argb[2]; if (fr > 255) fr = 255;
    return color_rgb(fr, fg, fb);
}

static color_t color_get_by_uv(struct shader_t *sd, vec2f_t uv)
{
    color_t c = { .c = texture_getcolor(sd->texture, uv.u * sd->texture->w, uv.v * sd->texture->h) };
    return c;
}

static void model_view_proj_transform(struct shader_t *sd, vertex_t t[3]) {
    for (int i = 0; i < 3; i++) {
        t[i].vn = mat4f_mul_vec4f(sd->mat_model, t[i].vn);
        t[i].v  = mat4f_mul_vec4f(sd->mat_model, t[i].v );
        t[i].v  = mat4f_mul_vec4f(sd->mat_view , t[i].v );
        t[i].v  = mat4f_mul_vec4f(sd->mat_proj , t[i].v );
    }
}

static int randcolor0_vertex(struct shader_t *sd, vertex_t t[3])
{
    model_view_proj_transform(sd, t);
    t[0].c = t[1].c = t[2].c = color_rgb(rand(), rand(), rand());
    for (int i = 0; i < 6; i++) rand();
    return 0;
}

static int randcolor1_vertex(struct shader_t *sd, vertex_t t[3])
{
    model_view_proj_transform(sd, t);
    for (int i = 0; i < 3; i++) t[i].c = color_rgb(rand(), rand(), rand());
    return 0;
}

static int flatcolor_vertex(struct shader_t *sd, vertex_t t[3])
{
    model_view_proj_transform(sd, t);
    vec3f_t v1 = vec3f_new(t[2].v.x - t[0].v.x, t[2].v.y - t[0].v.y, t[2].v.z - t[0].v.z);
    vec3f_t v2 = vec3f_new(t[1].v.x - t[0].v.x, t[1].v.y - t[0].v.y, t[1].v.z - t[0].v.z);
    vec3f_t vn = vec3f_normalize(vec3f_cross(v1, v2));
    float intensity = vec3f_dot(vn, sd->light);
    if (intensity < 0) return -1;
    for (int i = 0; i < 3; i++) t[i].c = color_intensity(sd->color, intensity);
    return 0;
}

static int gouraud_vertex(struct shader_t *sd, vertex_t t[3])
{
    model_view_proj_transform(sd, t);
    for (int i = 0; i < 3; i++) {
        float intensity = vec3f_dot(vec3f_from_vec4f(t[i].vn), sd->light);
        if (intensity < 0) return -1;
        t[i].c = color_intensity(sd->color, intensity);
    }
    return 0;
}

static int mvpp_vertex(struct shader_t *sd, vertex_t t[3])
{
    model_view_proj_transform(sd, t);
    return 0;
}

static int wireframe_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    if (t[0].w) {
        for (int i = 0; i < 3; i++) texture_line(sd->target, t[i].v.x, t[i].v.y, t[(i + 1) % 3].v.x, t[(i + 1) % 3].v.y, sd->color.c);
        t[0].w = 0;
    }
    return -1;
}

static int fillcolor0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc) { return t[0].c.c; }
static int fillcolor1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc) { return color_interpolate(t, perspective_correction(t, bc)).c; }

static int phongcolor_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    vec4f_t vn = normal_interpolate(t, perspective_correction(t, bc));
    float intensity = vec3f_dot(vec3f_from_vec4f(vn), sd->light);
    if (intensity < 0) return -1;
    return color_intensity(sd->color, intensity).c;
}

static int normal0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    return color_from_normal(t[0].vn).c;
}

static int normal1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    return color_from_normal(normal_interpolate(t, perspective_correction(t, bc))).c;
}

static int texture0_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    return color_get_by_uv(sd, uv_interpolate(t, perspective_correction(t, bc))).c;
}

static int texture1_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    float intensity = vec3f_dot(vec3f_from_vec4f(t[0].vn), sd->light);
    if (intensity < 0) return -1;
    return color_intensity(color_get_by_uv(sd, uv_interpolate(t, perspective_correction(t, bc))), intensity).c;
}

static int texture2_fragmt(struct shader_t *sd, vertex_t t[3], vec3f_t bc)
{
    vec3f_t pcbc = perspective_correction(t, bc);
    vec4f_t vn   = normal_interpolate(t, pcbc);
    float intensity = vec3f_dot(vec3f_from_vec4f(vn), sd->light);
    if (intensity < 0) return -1;
    return color_intensity(color_get_by_uv(sd, uv_interpolate(t, pcbc)), intensity).c;
}

static struct {
    char *name;      int (*vertex)(struct shader_t *sd, vertex_t t[3]);
} s_vlist[] = {
    { "rand0"      , randcolor0_vertex },
    { "rand1"      , randcolor1_vertex },
    { "flat"       , flatcolor_vertex  },
    { "gouraud"    , gouraud_vertex    },
    { "mvpp"       , mvpp_vertex       },
    { NULL         , NULL              },
};

static struct {
    char *name;      int (*fragmt)(struct shader_t *sd, vertex_t t[3], vec3f_t bc);
} s_flist[] = {
    { "wire"       , wireframe_fragmt  },
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
    sd->light .x =  0;
    sd->light .y = -0.5;
    sd->light .z = -1;
    sd->vertex   = flatcolor_vertex;
    sd->fragmt   = fillcolor0_fragmt;
    sd->mat_model = sd->mat_view = sd->mat_proj = mat4f_identity();
    set_vertex(sd, vertex);
    set_fragmt(sd, fragmt);
    return sd;
}

void shader_free(SHADER *sd) { if (sd) { texture_free(sd->deftext); free(sd); } }

void shader_set(SHADER *sd, char *name, void *data)
{
    if (!sd || !name) return;
    if      (strcmp(name, "mat_model") == 0) sd->mat_model = *(mat4f_t*)data;
    else if (strcmp(name, "mat_view" ) == 0) sd->mat_view  = *(mat4f_t*)data;
    else if (strcmp(name, "mat_proj" ) == 0) sd->mat_proj  = *(mat4f_t*)data;
    else if (strcmp(name, "target"   ) == 0) sd->target    = data;
    else if (strcmp(name, "texture"  ) == 0) sd->texture   = data;
    else if (strcmp(name, "color"    ) == 0) sd->color     = *(color_t*)data;
    else if (strcmp(name, "light"    ) == 0) sd->light     = *(vec3f_t*)data;
    else if (strcmp(name, "vertex"   ) == 0) set_vertex(sd, (char*)data);
    else if (strcmp(name, "fragmt"   ) == 0) set_fragmt(sd, (char*)data);
}

void* shader_get(SHADER *sd, char *name)
{
    if (!sd || !name) return NULL;
    if      (strcmp(name, "mat_model") == 0) return &sd->mat_model;
    else if (strcmp(name, "mat_view" ) == 0) return &sd->mat_view ;
    else if (strcmp(name, "mat_proj" ) == 0) return &sd->mat_proj ;
    else if (strcmp(name, "target"   ) == 0) return  sd->target;
    else if (strcmp(name, "texture"  ) == 0) return  sd->texture;
    else if (strcmp(name, "color"    ) == 0) return &sd->color;
    else if (strcmp(name, "light"    ) == 0) return &sd->light;
    else if (strcmp(name, "vertex"   ) == 0) return  sd->vertex;
    else if (strcmp(name, "fragmt"   ) == 0) return  sd->fragmt;
    return NULL;
}
