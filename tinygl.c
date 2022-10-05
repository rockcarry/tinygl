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
#include <math.h>
#include <unistd.h>
#include "matrix.h"
#include "wingdi.h"
int main(void)
{
    struct {
        char *file_obj;
        char *file_text;
        void *model;
    } s_model_list[] = {
        { "model/head.obj"          , "model/head.tga"           },
        { "model/head_eye_inner.obj", "model/head_eye_inner.tga" },
        { NULL                      , NULL                       },
    };
    void *win = wingdi_init(640, 640);
    void *gl  = tinygl_init(0  , 0  );
    int   angle = 0, i;

    tinygl_set(gl, "target", wingdi_get(win, "texture"));
    tinygl_set(gl, "shader.target", wingdi_get(win, "texture"));
    tinygl_set(gl, "shader.vertex", "flat");
    tinygl_set(gl, "shader.fragmt", "texture0");

    for (i = 0; s_model_list[i].file_obj; i++) s_model_list[i].model = model_load(s_model_list[i].file_obj, s_model_list[i].file_text);
    while (strcmp(wingdi_get(win, "state"), "closed") != 0) {
        mat4f_t matrot = mat4f_rotate_y(angle * 2 * M_PI / 360); angle += 2;
        tinygl_set(gl, "shader.mat_model", &matrot);
        tinygl_begin(gl, 1);
        for (i = 0; s_model_list[i].model; i++) {
            tinygl_set (gl, "shader.texture", model_get_texture(s_model_list[i].model));
            tinygl_draw(gl, s_model_list[i].model);
        }
        tinygl_end(gl);
        usleep(10 * 1000);
    }
    for (i = 0; s_model_list[i].file_obj; i++) { model_free(s_model_list[i].model); s_model_list[i].model = NULL; }

    tinygl_set (gl, "save", "out.bmp");
    tinygl_free(gl);
    wingdi_free(win, 0);
    return 0;
}
#endif
