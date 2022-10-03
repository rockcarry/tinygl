#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "matrix.h"
#include "triangle.h"
#include "model.h"
#include "shader.h"
#include "tinygl.h"

typedef struct {
    TEXTURE *screen;
    float   *zbuffer;
    void    *shader;
} TINYGL;

static void clearzbuffer(TINYGL *gl)
{
    int i, n = gl->screen->width * gl->screen->height;
    for (i = 0; i < n; i++) gl->zbuffer[i] = -FLT_MAX;
}

void* tinygl_init(int w, int h, void *shader)
{
    TINYGL *gl = malloc(sizeof(TINYGL) + w * h * sizeof(float));
    if (!gl) goto failed;

    gl->screen = texture_create(w, h, 24);
    if (!gl->screen) goto failed;

    gl->zbuffer = (float*)(gl + 1);
    gl->shader  = shader;
    clearzbuffer(gl);
    tinygl_set_viewport(gl, 0, 0, w, h, 255);
    return gl;

failed:
    tinygl_free(gl);
    return NULL;
}

void tinygl_free(void *ctx)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (gl) {
        texture_destroy(gl->screen);
        free(gl);
    }
}

void tinygl_draw(void *ctx, void *model)
{
    TINYGL  *gl = (TINYGL*)ctx;
    vertex_t triangle[3];
    int      nface, i;
    if (!gl) return;
    nface = model_get_face(model, -1, NULL);
    for (i = 0; i < nface; i++) {
        model_get_face(model, i, triangle);
        if (shader_vertex(gl->shader, triangle) == 0) draw_triangle (gl->screen, gl->zbuffer, gl->shader, triangle);
    }
}

void tinygl_clear(void *ctx, char *type)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx) return;
    if (strcmp(type, "screen") == 0) {
        texture_fillrect(gl->screen, 0, 0, gl->screen->width, gl->screen->height, 0);
    } else if (strcmp(type, "zbuffer") == 0) {
        clearzbuffer(gl);
    }
}

void tinygl_set_viewport(void *ctx, int x, int y, int w, int h, int depth)
{
    if (ctx) {
        float *matrix = tinygl_get_param(ctx, "port");
        matrix_identity(matrix , 4);
        matrix[0 * 4 + 3] = (float)x + w / 2.0;
        matrix[1 * 4 + 3] = (float)y + h / 2.0;
        matrix[2 * 4 + 3] = (float)depth / 2.0;
        matrix[0 * 4 + 0] = (float)w / 2.0;
        matrix[1 * 4 + 1] = (float)h /-2.0;
        matrix[2 * 4 + 2] = (float)depth / 2.0;
    }
}

void tinygl_set_param(void *ctx, char *name, void *data)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx || !name) return;
    if      (strcmp(name, "shader"    ) == 0) gl->shader = data;
    else if (strcmp(name, "savescreen") == 0) texture_save(gl->screen, data);
    else shader_set_param(gl->shader, name, data);
}

void* tinygl_get_param(void *ctx, char *name)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx || !name) return NULL;
    if      (strcmp(name, "shader") == 0) return gl->shader;
    else if (strcmp(name, "screen") == 0) return gl->screen;
    else return shader_get_param(ctx ? ((TINYGL*)ctx)->shader : NULL, name);
    return NULL;
}

#ifdef _TEST_TINYGL_
int main(void)
{
    void *shader = shader_init("flat", "normal0");
    void *tinygl = tinygl_init(1024, 1024, shader);
    void *model  = model_load ("head.obj", "head.bmp");

    tinygl_set_param(tinygl, "texture", model_get_texture(model));
    tinygl_draw(tinygl, model);
    tinygl_set_param(tinygl, "savescreen", "out.bmp");

    model_free (model );
    tinygl_free(tinygl);
    shader_free(shader);
    return 0;
}
#endif
