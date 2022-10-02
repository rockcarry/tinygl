#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "vector.h"
#include "texture.h"
#include "shader.h"

void draw_triangle(TEXTURE *dst, float *zbuf, void *shader, vertex_t v[3]);

#endif
