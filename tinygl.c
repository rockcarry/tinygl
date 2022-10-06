#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "matrix.h"
#include "triangle.h"
#include "model.h"
#include "shader.h"
#include "tinygl.h"

typedef struct {
    TEXTURE *target;
    TEXTURE *deftgt;
    SHADER  *shader;
    SHADER  *defshd;
    float   *zbuffer;
} TINYGL;

static int reinitzbuffer(TINYGL *gl, int zw, int zh)
{
    float *newzbuf = NULL;
    if (!gl->zbuffer || zw != gl->target->w || zh != gl->target->h) {
        newzbuf = malloc(zw * zh * sizeof(float));
        if (!newzbuf) return -1;
        free(gl->zbuffer);
        gl->zbuffer = newzbuf;
    }
    for (int i = 0, n = zw * zh; i < n; i++) gl->zbuffer[i] = -FLT_MAX;
    return 0;
}

void* tinygl_init(int w, int h)
{
    TINYGL *gl = calloc(1, sizeof(TINYGL));
    if (!gl) goto failed;

    gl->deftgt = texture_init(w, h);
    gl->defshd = shader_init (NULL, NULL);
    if (!gl->deftgt || !gl->defshd) goto failed;
    gl->target = gl->deftgt;
    gl->shader = gl->defshd;
    if (reinitzbuffer(gl, gl->target->w, gl->target->h) != 0) goto failed;
    tinygl_viewport(gl, 0, 0, w, h, 255);
    return gl;

failed:
    tinygl_free(gl);
    return NULL;
}

void tinygl_free(void *ctx)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (gl) {
        texture_free(gl->deftgt);
        shader_free (gl->defshd);
        free(gl->zbuffer);
        free(gl);
    }
}

void tinygl_begin(void *ctx, int clear)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (gl) {
        texture_lock(gl->target);
        texture_fillrect(gl->target, 0, 0, gl->target->w, gl->target->h, 0);
        reinitzbuffer(gl, gl->target->w, gl->target->h);
    }
}

void tinygl_end(void *ctx) { TINYGL *gl = (TINYGL*)ctx; if (gl) texture_unlock(gl->target); }

void tinygl_draw(void *ctx, void *m)
{
    TINYGL  *gl = (TINYGL*)ctx;
    vertex_t t[3];
    if (!gl) return;
    int nface = model_get_face(m, -1, NULL);
    for (int i = 0; i < nface; i++) {
        model_get_face(m, i, t);
        draw_triangle(gl->target, gl->zbuffer, gl->shader, t);
    }
}

void tinygl_clear(void *ctx, char *type)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx) return;
    if (strstr(type, "framebuf")) texture_fillrect(gl->target, 0, 0, gl->target->w, gl->target->h, 0);
    if (strcmp(type, "zbuffer" )) reinitzbuffer(gl, gl->target->w, gl->target->h);
}

void tinygl_viewport(void *ctx, int x, int y, int w, int h, int depth)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx) return;
    mat4f_t m = mat4f_viewport(x, y, w, h, depth);
    shader_set(gl->shader, "mat_port", &m);
}

void tinygl_set(void *ctx, char *name, void *data)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx || !name || !data) return;
    if      (strcmp(name, "shader" ) == 0   ) gl->shader = data;
    else if (strcmp(name, "target" ) == 0   ) {
        if (reinitzbuffer(gl, ((TEXTURE*)data)->w, ((TEXTURE*)data)->h) == 0) {
            gl->target = data; tinygl_viewport(gl, 0, 0, gl->target->w, gl->target->h, 255);
        }
    }
    else if (strstr(name, "shader.") == name) shader_set(gl->shader, name + 7, data);
    else if (strcmp(name, "save"   ) == 0   ) texture_save(gl->target, data);
}

void* tinygl_get(void *ctx, char *name)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx || !name) return NULL;
    if      (strcmp(name, "shader" ) == 0   ) return gl->shader;
    else if (strcmp(name, "target" ) == 0   ) return gl->target;
    else if (strstr(name, "shader.") == name) return shader_get(gl->shader, name + 7);
    return NULL;
}

#ifdef _TEST_TINYGL_
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "matrix.h"
#include "wingdi.h"
#include "utils.h"

static char *s_model_list[][2] = {
    { "model/head.obj"          , "model/head.tga"           },
    { "model/head_eye_inner.obj", "model/head_eye_inner.tga" },
};

static char *s_shader_list[][2] = {
    { "wire"   , "none"    }, // 线框
    { "rand"   , "color0"  }, // 随机颜色，色块填充
    { "rand"   , "color1"  }, // 随机颜色，渐变填充
    { "flat"   , "color0"  }, // flat 着色，颜色填充
    { "gouraud", "color1"  }, // gouraud 着色，颜色填充
    { "mvpp"   , "phong"   }, // phong 着色，颜色填充
    { "flat"   , "normal0" }, // flat 向量着色
    { "mvpp"   , "normal1" }, // phong 向量着色
    { "flat"   , "texture1"}, // flat 纹理映射
    { "mvpp"   , "texture0"}, // 直接纹理映射
    { "mvpp"   , "texture2"}, // phong 纹理映射
};

static void my_winmsg_callback(void *cbctx, int msg, uint32_t param1, uint32_t param2, void *param3)
{
    int *change = (int*)cbctx;
    switch (msg) {
    case WINGDI_MSG_KEY_EVENT:
        printf("key: %d, %s\n", param2, param1 ? "pressed" : "released"); fflush(stdout);
        if (param1 && param2 == ' ') *change = 1;
        break;
    }
}

int main(void)
{
    void *models[ARRAYSIZE(s_model_list)] = { NULL };
    int   angle = 0, curshader = 0, change = 1, i;
    void *win = wingdi_init(640, 640, my_winmsg_callback, &change);
    void *gl  = tinygl_init(0  , 0  );
    uint32_t fratectrl[4];

    tinygl_set(gl, "target", wingdi_get(win, "texture"));
    tinygl_set(gl, "shader.target", wingdi_get(win, "texture"));

    for (i = 0; i < ARRAYSIZE(models); i++) models[i] = model_load(s_model_list[i][0], s_model_list[i][1]);
    while (strcmp(wingdi_get(win, "state"), "closed") != 0) {
        if (change) {
            tinygl_set(gl, "shader.vertex", s_shader_list[curshader][0]);
            tinygl_set(gl, "shader.fragmt", s_shader_list[curshader][1]);
            change = 0, curshader = (curshader + 1) % ARRAYSIZE(s_shader_list);
        }
        mat4f_t matrot = mat4f_rotate_y(angle * 2 * M_PI / 360); angle += 2;
        tinygl_set(gl, "shader.mat_model", &matrot);
        tinygl_begin(gl, 1);
        for (i = 0; i < ARRAYSIZE(models); i++) {
            tinygl_set (gl, "shader.texture", model_get_texture(models[i]));
            tinygl_draw(gl, models[i]);
        }
        tinygl_end(gl);
        frame_rate_control(fratectrl, 60);
    }
    for (i = 0; i < ARRAYSIZE(models); i++) { model_free(models[i]); models[i] = NULL; }

    tinygl_set (gl, "save", "out.bmp");
    tinygl_free(gl);
    wingdi_free(win, 0);
    return 0;
}
#endif
