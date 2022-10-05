#include <math.h>
#include "triangle.h"

static vec3f_t barycentric(vertex_t v[3], int x, int y)
{
    vec3f_t va = { { v[2].v.x - v[0].v.x, v[1].v.x - v[0].v.x, v[0].v.x - x } };
    vec3f_t vb = { { v[2].v.y - v[0].v.y, v[1].v.y - v[0].v.y, v[0].v.y - y } };
    vec3f_t vs = vec3f_cross(va, vb);
    return vec3f_new(1.0 - (vs.x + vs.y) / vs.z, vs.y / vs.z, vs.x / vs.z);
}

void draw_triangle(TEXTURE *target, float *zbuf, SHADER *shader, vertex_t v[3])
{
    vec4f_t vmin, vmax; float z;
    int     iminx, iminy, imaxx, imaxy, c, i, j;

    if (shader->vertex(shader, v) != 0) return;
    for (vmin = vmax = v[0].v, i = 1; i < 3; i++) vmin = vec4f_min(vmin, v[i].v), vmax = vec4f_max(vmax, v[i].v);
    iminx = vmin.x > 0 ? floor(vmin.x) : 0;
    iminy = vmin.y > 0 ? floor(vmin.y) : 0;
    imaxx = ceil(vmax.x) < target->w - 1 ? ceil(vmax.x) : target->w - 1;
    imaxy = ceil(vmax.y) < target->h - 1 ? ceil(vmax.y) : target->h - 1;

    for (i = iminy; i <= imaxy; i++) {
        for (j = iminx; j <= imaxx; j++) {
            vec3f_t bc = barycentric(v, j, i);
            if (bc.alpha < 0 || bc.beta < 0 || bc.gamma < 0) continue;
            z = vec3f_dot(bc, vec3f_new(v[0].v.z, v[1].v.z, v[2].v.z));
            if (zbuf[j + i * target->w] < z) {
                if ((c = shader->fragmt(shader, v, bc)) != -1) {
                    texture_setcolor(target, j, i, c);
                    zbuf[j + i * target->w] = z;
                }
            }
        }
    }
}
