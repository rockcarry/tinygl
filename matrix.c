#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "matrix.h"

void matnf_identity(float *m, int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m[i * n + j] = (i == j) ? 1 : 0;
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

mat3f_t mat3f_identity(void)
{
    mat3f_t m = {{
        { 1, 0, 0, },
        { 0, 1, 0, },
        { 0, 0, 1, },
    }};
    return m;
}

mat3f_t mat3f_transpose(mat3f_t m) {
    mat3f_t t;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            t.m[i][j] = m.m[j][i];
        }
    }
    return t;
}

static float mat3f_determinant(mat3f_t m)
{
    float a = +m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
    float b = -m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    float c = +m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return a + b + c;
}

static mat3f_t mat3f_adjoint(mat3f_t m)
{
    mat3f_t adjoint;
    adjoint.m[0][0] = +(m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
    adjoint.m[0][1] = -(m.m[1][0] * m.m[2][2] - m.m[2][0] * m.m[1][2]);
    adjoint.m[0][2] = +(m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]);
    adjoint.m[1][0] = -(m.m[0][1] * m.m[2][2] - m.m[2][1] * m.m[0][2]);
    adjoint.m[1][1] = +(m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]);
    adjoint.m[1][2] = -(m.m[0][0] * m.m[2][1] - m.m[2][0] * m.m[0][1]);
    adjoint.m[2][0] = +(m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2]);
    adjoint.m[2][1] = -(m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
    adjoint.m[2][2] = +(m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1]);
    return adjoint;
}

mat3f_t mat3f_inverse_transpose(mat3f_t m)
{
    float   determinant = mat3f_determinant(m);
    mat3f_t adjoint     = mat3f_adjoint(m);
    mat3f_t result;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result.m[i][j] = adjoint.m[i][j] / determinant;
        }
    }
    return result;
}

mat3f_t mat3f_inverse(mat3f_t m)
{
    return mat3f_transpose(mat3f_inverse_transpose(m));
}

vec3f_t mat3f_mul_vec3f(mat3f_t m, vec3f_t v) {
    float tmp[3];
    for (int i = 0; i < 3; i++) {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        tmp[i] = a + b + c;
    }
    return vec3f_new(tmp[0], tmp[1], tmp[2]);
}

mat3f_t mat3f_mul_mat3f(mat3f_t a, mat3f_t b) {
    mat3f_t m = {{{0}}};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
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

mat4f_t mat4f_transpose(mat4f_t m)
{
    mat4f_t t;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            t.m[i][j] = m.m[j][i];
        }
    }
    return t;
}

static float mat4f_minor(mat4f_t m, int r, int c)
{
    mat3f_t cut_down;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int row = i < r ? i : i + 1;
            int col = j < c ? j : j + 1;
            cut_down.m[i][j] = m.m[row][col];
        }
    }
    return mat3f_determinant(cut_down);
}

static float mat4f_cofactor(mat4f_t m, int r, int c)
{
    float sign = (r + c) % 2 == 0 ? 1.0f : -1.0f;
    float minor = mat4f_minor(m, r, c);
    return sign * minor;
}

static mat4f_t mat4f_adjoint(mat4f_t m)
{
    mat4f_t adjoint;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            adjoint.m[i][j] = mat4f_cofactor(m, i, j);
        }
    }
    return adjoint;
}

mat4f_t mat4f_inverse_transpose(mat4f_t m)
{
    float   determinant = 0;
    mat4f_t adjoint = mat4f_adjoint(m);
    mat4f_t result;

    for (int i = 0; i < 4; i++) determinant += m.m[0][i] * adjoint.m[0][i];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = adjoint.m[i][j] / determinant;
        }
    }
    return result;
}

mat4f_t mat4f_inverse(mat4f_t m)
{
    return mat4f_transpose(mat4f_inverse_transpose(m));
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

mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b) {
    mat4f_t m = {{{0}}};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
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

mat4f_t mat4f_lookat(vec3f_t eye, vec3f_t target, vec3f_t up)
{
    vec3f_t z_axis = vec3f_normalize(vec3f_sub(eye, target));
    vec3f_t x_axis = vec3f_normalize(vec3f_cross(up, z_axis));
    vec3f_t y_axis = vec3f_cross(z_axis, x_axis);
    mat4f_t m      = mat4f_identity();

    m.m[0][0] = x_axis.x;
    m.m[0][1] = x_axis.y;
    m.m[0][2] = x_axis.z;

    m.m[1][0] = y_axis.x;
    m.m[1][1] = y_axis.y;
    m.m[1][2] = y_axis.z;

    m.m[2][0] = z_axis.x;
    m.m[2][1] = z_axis.y;
    m.m[2][2] = z_axis.z;

    m.m[0][3] = -vec3f_dot(x_axis, eye);
    m.m[1][3] = -vec3f_dot(y_axis, eye);
    m.m[2][3] = -vec3f_dot(z_axis, eye);

    return m;
}

mat4f_t mat4f_perspective(float fovy, float aspect, float near, float far)
{
    mat4f_t m = mat4f_identity();
    m.m[1][1] = 1 / tanf(fovy / 2);
    m.m[0][0] = m.m[1][1] / aspect;
    m.m[2][2] = 1 * (far + near) / (far - near);
    m.m[2][3] = 2 * (far * near) / (far - near);
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
    m.m[0][0] =(w - 1) / 2.0;
    m.m[1][1] =(h - 1) /-2.0;
    m.m[2][2] = depth / 2.0;
    return m;
}
