#include <math.h>
#include "triangle.h"

static void barycentric(vertex_t v[3], int x, int y, vec3f_t *bc)
{
    vec3f_t va = { { v[2].v.x - v[0].v.x, v[1].v.x - v[0].v.x, v[0].v.x - x } };
    vec3f_t vb = { { v[2].v.y - v[0].v.y, v[1].v.y - v[0].v.y, v[0].v.y - y } };
    vec3f_t vs; vector3f_cross((float*)&vs, (float*)&va, (float*)&vb);
    bc->alpha = 1.0 - (vs.x + vs.y) / vs.z;
    bc->beta  = vs.y / vs.z;
    bc->gamma = vs.x / vs.z;
}

void draw_triangle(TEXTURE *target, float *zbuf, SHADER *shader, vertex_t v[3])
{
    int   iminx, iminy, imaxx, imaxy, c, i, j;
    float fminx, fminy, fmaxx, fmaxy, z;

    for (fminx = fmaxx = v[0].v.x, fminy = fmaxy = v[0].v.y, i = 1; i < 3; i++) {
        if (fminx > v[i].v.x) fminx = v[i].v.x;
        if (fmaxx < v[i].v.x) fmaxx = v[i].v.x;
        if (fminy > v[i].v.y) fminy = v[i].v.y;
        if (fmaxy < v[i].v.y) fmaxy = v[i].v.y;
    }
    if (fmaxx > target->w - 1) fmaxx = target->w - 1;
    if (fmaxy > target->h - 1) fmaxy = target->h - 1;
    if (fminx < 0) fminx = 0;
    if (fminy < 0) fminy = 0;

    for (iminy = floor(fminy), imaxy = ceil (fmaxy), i = iminy; i <= imaxy; i++) {
        for (iminx = floor(fminx), imaxx = ceil (fmaxx), j = iminx; j <= imaxx; j++) {
            vec3f_t bc; barycentric(v, j, i, &bc);
            if (bc.alpha < 0 || bc.beta < 0 || bc.gamma < 0) continue;
            z = bc.alpha * v[0].v.z + bc.beta * v[1].v.z + bc.gamma * v[2].v.z;
            if (zbuf[j + i * target->w] < z) {
                if ((c = shader->fragmt(shader, v, &bc)) != -1) {
                    texture_setcolor(target, j, i, c);
                    zbuf[j + i * target->w] = z;
                }
            }
        }
    }
}
