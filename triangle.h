#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "vector.h"
#include "texture.h"
#include "shader.h"

void draw_triangle(TEXTURE *target, float *zbuf, SHADER *shader, vertex_t v[3]);

#endif
