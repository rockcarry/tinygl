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
    int    i, n;
    if (!gl->zbuffer || zw != gl->target->w || zh != gl->target->h) {
        newzbuf = malloc(zw * zh * sizeof(float));
        if (!newzbuf) return -1;
        free(gl->zbuffer);
        gl->zbuffer = newzbuf;
    }
    n = zw * zh;
    for (i = 0; i < n; i++) gl->zbuffer[i] = -FLT_MAX;
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

void tinygl_begin(void *ctx)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (gl) texture_lock(gl->target);
}

void tinygl_end(void *ctx)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (gl) texture_unlock(gl->target);
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
        if (gl->shader->vertex(gl->shader, triangle) == 0) draw_triangle(gl->target, gl->zbuffer, gl->shader, triangle);
    }
}

void tinygl_clear(void *ctx, char *type)
{
    TINYGL *gl = (TINYGL*)ctx;
    if (!ctx) return;
    if (strcmp(type, "framebuf") == 0) {
        texture_fillrect(gl->target, 0, 0, gl->target->w, gl->target->h, 0);
    } else if (strcmp(type, "zbuffer") == 0) {
        reinitzbuffer(gl, gl->target->w, gl->target->h);
    }
}

void tinygl_viewport(void *ctx, int x, int y, int w, int h, int depth)
{
    if (ctx) {
        float *matrix = tinygl_get(ctx, "shader.port");
        matrix_identity(matrix , 4);
        matrix[0 * 4 + 3] = (float)x + w / 2.0;
        matrix[1 * 4 + 3] = (float)y + h / 2.0;
        matrix[2 * 4 + 3] = (float)depth / 2.0;
        matrix[0 * 4 + 0] = (float)w / 2.0;
        matrix[1 * 4 + 1] = (float)h /-2.0;
        matrix[2 * 4 + 2] = (float)depth / 2.0;
    }
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
        { "model/head_eye_outer.obj", "model/head_eye_inner.tga" },
        { NULL                      , NULL                       },
    };
    void *wingdi = wingdi_init(800, 800);
    void *tinygl = tinygl_init(0  , 0  );
    int   i;

    tinygl_set(tinygl, "target", wingdi_get(wingdi, "texture"));
    tinygl_set(tinygl, "shader.vertex", "perspective");
    tinygl_set(tinygl, "shader.fragmt", "texture2"   );
    tinygl_begin(tinygl);
    for (i = 0; s_model_list[i].file_obj; i++) {
        s_model_list[i].model = model_load(s_model_list[i].file_obj, s_model_list[i].file_text);
        tinygl_set (tinygl, "shader.texture", model_get_texture(s_model_list[i].model));
        tinygl_draw(tinygl, s_model_list[i].model);
        model_free(s_model_list[i].model);
        s_model_list[i].model = NULL;
    }
    tinygl_end(tinygl);

    tinygl_set (tinygl, "save", "out.bmp");
    tinygl_free(tinygl);
    wingdi_free(wingdi, 0);
    return 0;
}
#endif
