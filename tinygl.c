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
#include "tinygl.h"
#include "model.h"
int main(void)
{
    void *gl = tinygl_init(800, 800);
    void *m  = model_load("model/head.obj", NULL);

    tinygl_begin(gl, 1);
    tinygl_draw(gl, m);
    tinygl_end(gl);

    tinygl_set(gl, "save", "out.bmp");
    model_free(m);
    tinygl_free(gl);
    return 0;
}
#endif
