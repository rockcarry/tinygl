#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>

typedef union {
    uint32_t c;
    uint8_t  argb[4];
} color_t;

typedef union {
    struct { int x, y; };
    struct { int u, v; };
} vec2i_t;

typedef union {
    struct { int x, y, z; };
} vec3i_t;

typedef union {
    struct { int x, y, z, w; };
} vec4i_t;

typedef union {
    struct { float x, y; };
    struct { float u, v; };
} vec2f_t;

typedef union {
    struct { float x, y, z; };
    struct { float u, v, w; };
    struct { float alpha, beta, gamma; };
} vec3f_t;

typedef union {
    struct { float x, y, z, w; };
    struct { float u, v, m, n; };
    struct { float alpha, beta, gamma, delta; };
} vec4f_t;

typedef struct {
    vec4f_t v;  // 顶点坐标
    vec2f_t vt; // 纹理坐标
    vec4f_t vn; // 法向量
    color_t c ; // 颜色值
} vertex_t;

color_t color_rgb (uint8_t r, uint8_t g, uint8_t b);
color_t color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

void  vecni_min(int *r, int *a, int *b, int n);
void  vecni_max(int *r, int *a, int *b, int n);
void  vecni_add(int *r, int *a, int *b, int n);
void  vecni_sub(int *r, int *a, int *b, int n);
void  vecni_mul(int *r, int *a, int  k, int n);
int   vecni_dot(int *a, int *b, int  n);
int   vecni_len(int *a, int  n);

void  vecnf_min(float *r, float *a, float *b, int n);
void  vecnf_max(float *r, float *a, float *b, int n);
void  vecnf_add(float *r, float *a, float *b, int n);
void  vecnf_sub(float *r, float *a, float *b, int n);
void  vecnf_mul(float *r, float *a, float  k, int n);
float vecnf_dot(float *a, float *b, int n);
float vecnf_len(float *a, int n);
void  vecnf_normalize(float *a, int n);

vec3f_t vec3f_new(float x, float y, float z);
vec3f_t vec3f_from_vec4f(vec4f_t v);
vec3f_t vec3f_min(vec3f_t a, vec3f_t b);
vec3f_t vec3f_max(vec3f_t a, vec3f_t b);
vec3f_t vec3f_add(vec3f_t a, vec3f_t b);
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b);
vec3f_t vec3f_mul(vec3f_t a, float   k);
float   vec3f_dot(vec3f_t a, vec3f_t b);
float   vec3f_len(vec3f_t a);
vec3f_t vec3f_normalize(vec3f_t a);
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b);

vec4f_t vec4f_new(float x, float y, float z, float w);
vec4f_t vec4f_from_vec3f(vec3f_t a, float w);
vec4f_t vec4f_min(vec4f_t a, vec4f_t b);
vec4f_t vec4f_max(vec4f_t a, vec4f_t b);
vec4f_t vec4f_add(vec4f_t a, vec4f_t b);
vec4f_t vec4f_sub(vec4f_t a, vec4f_t b);
vec4f_t vec4f_mul(vec4f_t a, float   k);
float   vec4f_dot(vec4f_t a, vec4f_t b);
float   vec4f_len(vec4f_t a);
vec4f_t vec4f_normalize(vec4f_t a);

#endif
