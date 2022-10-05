#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "matrix.h"

void matnf_mul(float *mr, float *ma, int rowa, int cola, float *mb, int rowb, int colb)
{
    for (int i = 0; i < rowa; i++) {
        for (int j = 0; j < colb; j++) {
            float sum = 0;
            for (int k = 0; k < cola; k++) {
                sum += ma[i * cola + k] * mb[k * colb + j];
            }
            mr[i * colb + j] = sum;
        }
    }
}

void matnf_transpose(float *mr, int rowr, int colr, float *ma, int rowa, int cola)
{
    for (int i = 0; i < rowr; i++) {
        for (int j = 0; j < colr; j++) {
            mr[i * colr + j] = ma[j * cola + i];
        }
    }
}

void matnf_identity(float *m, int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m[i * n + j] = (i == j) ? 1 : 0;
        }
    }
}

mat3f_t mat3f_identity(void)
{
    mat3f_t m = {{
        { 1, 0, 0, },
        { 0, 1, 0, },
        { 0, 0, 1, },
    }};
    return m;
}

mat4f_t mat4f_identity(void)
{
    mat4f_t m = {{
        { 1, 0, 0, 0, },
        { 0, 1, 0, 0, },
        { 0, 0, 1, 0, },
        { 0, 0, 0, 1, },
    }};
    return m;
}

vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v)
{
    float tmp[4];
    for (int i = 0; i < 4; i++) {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        float d = m.m[i][3] * v.w;
        tmp[i]  = a + b + c + d;
    }
    return vec4f_new(tmp[0], tmp[1], tmp[2], tmp[3]);
}

mat4f_t mat4f_translate(vec3f_t t)
{
    mat4f_t m = mat4f_identity();
    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;
    return m;
}

mat4f_t mat4f_scale(vec3f_t f, vec3f_t s)
{
    mat4f_t m = mat4f_identity();
    m.m[0][0] = s.x;
    m.m[1][1] = s.y;
    m.m[2][2] = s.z;
    m.m[3][0] = (1 - s.x) * f.x;
    m.m[3][1] = (1 - s.y) * f.y;
    m.m[3][2] = (1 - s.z) * f.z;
    return m;
}

mat4f_t mat4f_rotate_x(float arc)
{
    float s = sinf(arc), c = cosf(arc);
    mat4f_t m = mat4f_identity();
    m.m[1][1] = c;
    m.m[1][2] = s;
    m.m[2][1] =-s;
    m.m[2][2] = c;
    return m;
}

mat4f_t mat4f_rotate_y(float arc)
{
    float s = sinf(arc), c = cosf(arc);
    mat4f_t m = mat4f_identity();
    m.m[0][0] = c;
    m.m[0][2] =-s;
    m.m[2][0] = s;
    m.m[2][2] = c;
    return m;
}

mat4f_t mat4f_rotate_z(float arc)
{
    float s = sinf(arc), c = cosf(arc);
    mat4f_t m = mat4f_identity();
    m.m[0][0] = c;
    m.m[0][1] = s;
    m.m[1][0] =-s;
    m.m[1][1] = c;
    return m;
}

mat4f_t mat4f_rotate_v(float arc, vec3f_t v)
{
    float   s = sinf(arc), c = cosf(arc);
    mat4f_t m = mat4f_identity();

    v = vec3f_normalize(v);
    m.m[0][0] = v.x * v.x * (1 - c) + c;
    m.m[0][1] = v.y * v.x * (1 - c) - s * v.z;
    m.m[0][2] = v.z * v.x * (1 - c) + s * v.y;

    m.m[1][0] = v.x * v.y * (1 - c) + s * v.z;
    m.m[1][1] = v.y * v.y * (1 - c) + c;
    m.m[1][2] = v.z * v.y * (1 - c) - s * v.x;

    m.m[2][0] = v.x * v.z * (1 - c) - s * v.y;
    m.m[2][1] = v.y * v.z * (1 - c) + s * v.x;
    m.m[2][2] = v.z * v.z * (1 - c) + c;
    return m;
}

mat4f_t mat4f_perspective(float fovy, float aspect, float near, float far)
{
    float z_range = far - near;
    mat4f_t m = mat4f_identity();
    m.m[1][1] = 1 / tanf(fovy / 2);
    m.m[0][0] = m.m[1][1] / aspect;
    m.m[2][2] =-(near + far) / z_range;
    m.m[2][3] =-2 * near * far / z_range;
    m.m[3][2] =-1;
    m.m[3][3] = 0;
    return m;
}

mat4f_t mat4f_viewport(int x, int y, int w, int h, int depth)
{
    mat4f_t m = mat4f_identity();
    m.m[0][3] = x + w / 2.0;
    m.m[1][3] = y + h / 2.0;
    m.m[2][3] = depth / 2.0;
    m.m[0][0] = w / 2.0;
    m.m[1][1] = h /-2.0;
    m.m[2][2] = depth / 2.0;
    return m;
}
