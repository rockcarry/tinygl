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
    vec4f_t vmin, vmax; int i, j, c;
    for (vmin = vmax = v[0].v, i = 1; i < 3; i++) vmin = vec4f_min(vmin, v[i].v), vmax = vec4f_max(vmax, v[i].v);
    int iminx = vmin.x > 0 ? floor(vmin.x) : 0;
    int iminy = vmin.y > 0 ? floor(vmin.y) : 0;
    int imaxx = ceil(vmax.x) < target->w - 1 ? ceil(vmax.x) : target->w - 1;
    int imaxy = ceil(vmax.y) < target->h - 1 ? ceil(vmax.y) : target->h - 1;

    for (i = iminy; i <= imaxy; i++) {
        for (j = iminx; j <= imaxx; j++) {
            vec3f_t bc = barycentric(v, j, i);
            if (bc.alpha < 0 || bc.beta < 0 || bc.gamma < 0) continue;
            float z = vec3f_dot(bc, vec3f_new(v[0].v.z, v[1].v.z, v[2].v.z));
            if (zbuf[j + i * target->w] < z) {
                if ((c = shader->fragmt(shader, v, bc)) != -1) {
                    target->data[i * target->w + j] = c;
                    zbuf[j + i * target->w] = z;
                }
            }
        }
    }
}
