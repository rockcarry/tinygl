#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "bmp.h"
#include "geometry.h"

void triangle(BMP *pb, BMP *img, float *zbuf, vec3f_t vv[3], vec3f_t vt[3], float intensity, int c);

#endif
