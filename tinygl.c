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
    mat4f_t  matvport;
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

#define CLIP_W_EPSILON (1e-5f)
static int is_inside_plane(int plane, vec4f_t coord) {
    switch (plane) {
    case  0: return coord.w >= CLIP_W_EPSILON; //  w
    case  1: return coord.x <= +coord.w; //  x
    case  2: return coord.x >= -coord.w; // -x
    case  3: return coord.y <= +coord.w; //  y
    case  4: return coord.y >= -coord.w; // -y
    case  5: return coord.z <= +coord.w; //  z
    case  6: return coord.z >= -coord.w; // -z
    default: return 0;
    }
}

static float get_interpolation(int plane, vec4f_t pre, vec4f_t cur)
{
    switch (plane) {
    case  0: return (pre.w - CLIP_W_EPSILON) / (pre.w - cur.w);
    case  1: return (pre.w - pre.x) / ((pre.w - pre.x) - (cur.w - cur.x));
    case  2: return (pre.w + pre.x) / ((pre.w + pre.x) - (cur.w + cur.x));
    case  3: return (pre.w - pre.y) / ((pre.w - pre.y) - (cur.w - cur.y));
    case  4: return (pre.w + pre.y) / ((pre.w + pre.y) - (cur.w + cur.y));
    case  5: return (pre.w - pre.z) / ((pre.w - pre.z) - (cur.w - cur.z));
    case  6: return (pre.w + pre.z) / ((pre.w + pre.z) - (cur.w + cur.z));
    default: return 0;
    }
}

static int clip_with_plane(int plane, vertex_t *vlist, int vnum, vertex_t *ilist)
{
    int  i, ipre, in_pre, in_cur, inum = 0;
    for (i = 0; i < vnum; i++) {
        ipre = (i + (vnum - 1)) % vnum;
        vertex_t vpre = vlist[ipre];
        vertex_t vcur = vlist[i   ];
        in_pre = is_inside_plane(plane, vpre.v);
        in_cur = is_inside_plane(plane, vcur.v);
        if (in_pre != in_cur) {
            float t = get_interpolation(plane, vpre.v, vcur.v);
            ilist[inum  ].v = vec4f_lerp(vpre.v , vcur.v , t);
            ilist[inum  ].vn= vec4f_lerp(vpre.vn, vcur.vn, t);
            ilist[inum  ].vt= vec2f_lerp(vpre.vt, vcur.vt, t);
            ilist[inum++].c = color_lerp(vpre.c , vcur.c , t);
        }
        if (in_cur) ilist[inum++] = vcur;
    }
    return inum;
}

static vec4f_t perspective_division(vec4f_t v) { return vec4f_new(v.x / v.w, v.y / v.w, v.z / v.w, 1); }

static int is_back_facing(vertex_t v[3]) {
    vec3f_t a = {{ v[0].v.x, v[0].v.y, v[0].v.z }};
    vec3f_t b = {{ v[1].v.x, v[1].v.y, v[1].v.z }};
    vec3f_t c = {{ v[2].v.x, v[2].v.y, v[2].v.z }};
    float signed_area = a.x * b.y - a.y * b.x
                      + b.x * c.y - b.y * c.x
                      + c.x * a.y - c.y * a.x;
    return signed_area <= 0;
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
        if (clear) {
            texture_fillrect(gl->target, 0, 0, gl->target->w, gl->target->h, 0);
            reinitzbuffer(gl, gl->target->w, gl->target->h);
        }
    }
}

void tinygl_end(void *ctx) { TINYGL *gl = (TINYGL*)ctx; if (gl) texture_unlock(gl->target); }

void tinygl_draw(void *ctx, void *m)
{
    TINYGL  *gl = (TINYGL*)ctx;
    vertex_t ft[3], ct[3];
    vertex_t vlist1[8];
    vertex_t vlist2[8];
    int      i, j, k, n;
    if (!gl) return;
    int nface = model_get_face(m, -1, NULL);
    for (i = 0; i < nface; i++) {
        model_get_face(m, i, ft);
        if (gl->shader->vertex(gl->shader, ft) == 0) {
            n = clip_with_plane(0, ft    , 3, vlist1);
            n = clip_with_plane(1, vlist1, n, vlist2);
            n = clip_with_plane(2, vlist2, n, vlist1);
            n = clip_with_plane(3, vlist1, n, vlist2);
            n = clip_with_plane(4, vlist2, n, vlist1);
            n = clip_with_plane(5, vlist1, n, vlist2);
            n = clip_with_plane(6, vlist2, n, vlist1);
            for (j = 0; j < n - 2; j++) {
                ct[0] = vlist1[0];
                ct[1] = vlist1[j + 1];
                ct[2] = vlist1[j + 2];
                for (k = 0; k < 3; k++) {
                    ct[k].w = ct[k].v.w;
                    ct[k].v = perspective_division(ct[k].v);
                }
                if (is_back_facing(ct)) continue;
                for (k = 0; k < 3; k++) ct[k].v = mat4f_mul_vec4f(gl->matvport, ct[k].v);
                draw_triangle(gl->target, gl->zbuffer, gl->shader, ct);
            }
        }
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
    gl->matvport = mat4f_viewport(x, y, w, h, depth);
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
