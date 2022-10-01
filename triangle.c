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

void triangle(BMP *pb, BMP *img, float *zbuf, vec3f_t vv[3], vec3f_t vt[3], float intensity, int c)
{
    int     iminx, iminy, imaxx, imaxy, i, j;
    float   fminx, fminy, fmaxx, fmaxy;
    uint8_t r, g, b;

    for (fminx = fmaxx = vv[0].x, fminy = fmaxy = vv[0].y, i = 1; i < 3; i++) {
        if (fminx > vv[i].x) fminx = vv[i].x;
        if (fmaxx < vv[i].x) fmaxx = vv[i].x;
        if (fminy > vv[i].y) fminy = vv[i].y;
        if (fmaxy < vv[i].y) fmaxy = vv[i].y;
    }

    for (iminy = floor(fminy), imaxy = ceil (fmaxy), i = iminy; i <= imaxy; i++) {
        for (iminx = floor(fminx), imaxx = ceil (fmaxx), j = iminx; j <= imaxx; j++) {
            vec3f_t bc; barycentric(vv, j, i, &bc);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            if (zbuf) {
                float z = bc.x * vv[0].z + bc.y * vv[1].z + bc.z * vv[2].z;
                float u = bc.x * vt[0].u + bc.y * vt[1].u + bc.z * vt[2].u;
                float v = bc.x * vt[0].v + bc.y * vt[1].v + bc.z * vt[2].v;
                if (zbuf[j + i * pb->width] < z) {
                    zbuf[j + i * pb->width] = z;
                    if (img) {
                        bmp_getrgb(img, u * img->width, v * img->height, &r, &g, &b);
                        bmp_setrgb(pb, j, i, r * intensity, g * intensity, b * intensity);
                    } else {
                        bmp_setpixel(pb, j, i, c);
                    }
                }
            } else {
                bmp_setpixel(pb, j, i, c);
            }
        }
    }
}
