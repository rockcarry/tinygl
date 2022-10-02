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
    model_get_data(model, MODEL_DATA_LIST_F, &nface);
    for (i = 0; i < nface; i++) {
        model_get_face(model, i, triangle);
        shader_vertex (gl->shader, triangle, 3);
        draw_triangle (gl->screen, gl->zbuffer, gl->shader, triangle);
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

void tinygl_save_screen(void *ctx, char *file)
{
    if (ctx) texture_save(((TINYGL*)ctx)->screen, file);
}

void* tinygl_get_screen(void *ctx)
{
     return ctx ? ((TINYGL*)ctx)->screen : NULL;
}

void tinygl_set_matrix(void *ctx, char *type, float *matrix)
{
    if (ctx) shader_set_matrix(((TINYGL*)ctx)->shader, type, matrix);
}

void* tinygl_get_matrix(void *ctx, char *type)
{
    return shader_get_matrix(ctx ? ((TINYGL*)ctx)->shader : NULL, type);
}

void tinygl_set_shader(void *ctx, void *shader)
{
    if (ctx) ((TINYGL*)ctx)->shader = shader;
}

void tinygl_set_viewport(void *ctx, int x, int y, int w, int h, int depth)
{
    float *matrix;
    if (!ctx) return;
    matrix = tinygl_get_matrix(ctx, "port");
    matrix_identity(matrix , 4);
    matrix[0 * 4 + 3] = (float)x + w / 2.0;
    matrix[1 * 4 + 3] = (float)y + h / 2.0;
    matrix[2 * 4 + 3] = (float)depth / 2.0;
    matrix[0 * 4 + 0] = (float)w / 2.0;
    matrix[1 * 4 + 1] = (float)h /-2.0;
    matrix[2 * 4 + 2] = (float)depth / 2.0;
}

#ifdef _TEST_TINYGL_
int main(void)
{
    void *shader= shader_init("randcolor", "interpcolor");
    void *gl    = tinygl_init(1024, 1024, shader);
    void *model = model_load("head.obj", "head.bmp");

    shader_set_param(shader, "screen", tinygl_get_screen(gl));
    tinygl_draw(gl, model);
    tinygl_save_screen(gl, "out.bmp");

    model_free(model);
    shader_free(shader);
    tinygl_free(gl);
    return 0;
}
#endif
