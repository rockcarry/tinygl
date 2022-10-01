#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "triangle.h"

static void barycentric(vec3f_t t[3], int x, int y, vec3f_t *bc)
{
    vec3f_t va = { { { t[2].x - t[0].x, t[1].x - t[0].x, t[0].x - x } } };
    vec3f_t vb = { { { t[2].y - t[0].y, t[1].y - t[0].y, t[0].y - y } } };
    vec3f_t vs;
    vector3f_cross((float*)&vs, (float*)&va, (float*)&vb);
    bc->x = 1.0 - (vs.x + vs.y) / vs.z;
    bc->y = vs.y / vs.z;
    bc->z = vs.x / vs.z;
}

void triangle(BMP *pb, float *zbuf, vec3f_t t[3], int c, int fill)
{
    int   iminx, iminy, imaxx, imaxy, i, j;
    float fminx, fminy, fmaxx, fmaxy;

    if (!fill) {
        bmp_line(pb, t[0].x, t[0].y, t[1].x, t[1].y, c);
        bmp_line(pb, t[1].x, t[1].y, t[2].x, t[2].y, c);
        bmp_line(pb, t[2].x, t[2].y, t[0].x, t[0].y, c);
        return;
    }

    for (fminx = fmaxx = t[0].x, fminy = fmaxy = t[0].y, i = 1; i < 3; i++) {
        if (fminx > t[i].x) fminx = t[i].x;
        if (fmaxx < t[i].x) fmaxx = t[i].x;
        if (fminy > t[i].y) fminy = t[i].y;
        if (fmaxy < t[i].y) fmaxy = t[i].y;
    }

    for (iminy = floor(fminy), imaxy = ceil (fmaxy), i = iminy; i <= imaxy; i++) {
        for (iminx = floor(fminx), imaxx = ceil (fmaxx), j = iminx; j <= imaxx; j++) {
            vec3f_t bc; barycentric(t, j, i, &bc);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            if (zbuf) {
                float z = bc.x * t[0].z + bc.y * t[1].z + bc.z * t[2].z;
                if (zbuf[j + i * pb->width] < z) {
                    zbuf[j + i * pb->width] = z;
                    bmp_setpixel(pb, j, i, c);
                }
            } else {
                bmp_setpixel(pb, j, i, c);
            }
        }
    }
}
