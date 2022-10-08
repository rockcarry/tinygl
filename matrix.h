#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "vector.h"

typedef struct { float m[3][3]; } mat3f_t;
typedef struct { float m[4][4]; } mat4f_t;

#define matnf_add(mr, ma, mb, row, col) vecnf_add(mr, ma, mb, row * col)
#define matnf_sub(mr, ma, mb, row, col) vecnf_sub(mr, ma, mb, row * col)

void matnf_identity(float *m, int n);
void matnf_transpose(float *mr, int rowr, int colr, float *ma, int rowa, int cola);
void matnf_mul(float *mr, float *ma, int rowa, int cola, float *mb, int rowb, int colb);

mat3f_t mat3f_identity(void);
mat3f_t mat3f_transpose(mat3f_t m);
mat3f_t mat3f_inverse_transpose(mat3f_t m);
mat3f_t mat3f_inverse(mat3f_t m);
vec3f_t mat3f_mul_vec3f(mat3f_t m, vec3f_t v);
mat3f_t mat3f_mul_mat3f(mat3f_t a, mat3f_t b);

mat4f_t mat4f_identity(void);
mat4f_t mat4f_transpose(mat4f_t m);
mat4f_t mat4f_inverse_transpose(mat4f_t m);
mat4f_t mat4f_inverse(mat4f_t m);
vec4f_t mat4f_mul_vec4f(mat4f_t m, vec4f_t v);
mat4f_t mat4f_mul_mat4f(mat4f_t a, mat4f_t b);

mat4f_t mat4f_translate(vec3f_t t);
mat4f_t mat4f_scale(vec3f_t f, vec3f_t s);

mat4f_t mat4f_rotate_x(float arc);
mat4f_t mat4f_rotate_y(float arc);
mat4f_t mat4f_rotate_z(float arc);
mat4f_t mat4f_rotate_v(float arc, vec3f_t v);

mat4f_t mat4f_lookat(vec3f_t eye, vec3f_t target, vec3f_t up);
mat4f_t mat4f_perspective(float fovy, float aspect, float near, float far);
mat4f_t mat4f_viewport(int x, int y, int w, int h, int depth);

#endif

