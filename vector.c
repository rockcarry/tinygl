#include <math.h>
#include "vector.h"
#include "utils.h"

#define VECTOR_MIN(r, a, b, n) do { int  i; for (i = 0; i < n; i++) (r)[i] = MIN((a)[i], (b)[i]); } while (0)
#define VECTOR_MAX(r, a, b, n) do { int  i; for (i = 0; i < n; i++) (r)[i] = MAX((a)[i], (b)[i]); } while (0)
#define VECTOR_ADD(r, a, b, n) do { int  i; for (i = 0; i < n; i++) (r)[i] = (a)[i] + (b)[i];     } while (0)
#define VECTOR_SUB(r, a, b, n) do { int  i; for (i = 0; i < n; i++) (r)[i] = (a)[i] - (b)[i];     } while (0)
#define VECTOR_MUL(r, a, k, n) do { int  i; for (i = 0; i < n; i++) (r)[i] = (a)[i] * k;          } while (0)
#define VECTOR_DOT(r, a, b, n) do { int  i; for (i = 0; i < n; i++) (r)   += (a)[i] * (b)[i];     } while (0)
#define VECTOR_LEN(r, a, n   ) do { int  i; for (i = 0; i < n; i++) (r)   += (a)[i] * (a)[i];     } while (0)

void  vecni_min(int *r, int *a, int *b, int n) { VECTOR_MIN(r, a, b, n); }
void  vecni_max(int *r, int *a, int *b, int n) { VECTOR_MAX(r, a, b, n); }
void  vecni_add(int *r, int *a, int *b, int n) { VECTOR_ADD(r, a, b, n); }
void  vecni_sub(int *r, int *a, int *b, int n) { VECTOR_SUB(r, a, b, n); }
void  vecni_mul(int *r, int *a, int  k, int n) { VECTOR_MUL(r, a, k, n); }
int   vecni_dot(int *a, int *b, int n)         { int r = 0; VECTOR_DOT(r, a, b, n); return r; }
int   vecni_len(int *a, int  n)                { int r = 0; VECTOR_LEN(r, a, n);    return r; }

void  vecnf_min(float *r, float *a, float *b, int n) { VECTOR_MIN(r, a, b, n); }
void  vecnf_max(float *r, float *a, float *b, int n) { VECTOR_MAX(r, a, b, n); }
void  vecnf_add(float *r, float *a, float *b, int n) { VECTOR_ADD(r, a, b, n); }
void  vecnf_sub(float *r, float *a, float *b, int n) { VECTOR_SUB(r, a, b, n); }
void  vecnf_mul(float *r, float *a, float  k, int n) { VECTOR_MUL(r, a, k, n); }
float vecnf_dot(float *a, float *b, int n)           { float r = 0; VECTOR_DOT(r, a, b, n); return r;        }
float vecnf_len(float *a, int    n)                  { float r = 0; VECTOR_LEN(r, a, n);    return sqrtf(r); }
void  vecnf_normalize(float *a, int n)               { vecnf_mul(a, a, 1 / vecnf_len(a, n), n);              }

color_t color_rgb (uint8_t r, uint8_t g, uint8_t b)            { color_t c; c.argb[0] = b; c.argb[1] = g; c.argb[2] = r; c.argb[3] = 0; return c; }
color_t color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { color_t c; c.argb[0] = b; c.argb[1] = g; c.argb[2] = r; c.argb[3] = a; return c; }

vec3f_t vec3f_new(float x, float y, float z) { vec3f_t v = {{ x  , y  , z   }}; return v; }
vec3f_t vec3f_from_vec4f(vec4f_t a)          { vec3f_t v = {{ a.x, a.y, a.z }}; return v; }
vec3f_t vec3f_min(vec3f_t a, vec3f_t b)      { vec3f_t v = {{ MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z) }}; return v; }
vec3f_t vec3f_max(vec3f_t a, vec3f_t b)      { vec3f_t v = {{ MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z) }}; return v; }
vec3f_t vec3f_add(vec3f_t a, vec3f_t b)      { vec3f_t v = {{    (a.x+ b.x),    (a.y+ b.y),    (a.z+ b.z) }}; return v; }
vec3f_t vec3f_sub(vec3f_t a, vec3f_t b)      { vec3f_t v = {{    (a.x- b.x),    (a.y- b.y),    (a.z- b.z) }}; return v; }
vec3f_t vec3f_mul(vec3f_t a, float   k)      { vec3f_t v = {{    (a.x* k  ),    (a.y* k  ),    (a.z* k  ) }}; return v; }
float   vec3f_dot(vec3f_t a, vec3f_t b)      { return      (a.x * b.x + a.y * b.y + a.z * b.z); }
float   vec3f_len(vec3f_t a)                 { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }
vec3f_t vec3f_normalize(vec3f_t a)           { return vec3f_mul(a, 1 / vec3f_len(a)); }
vec3f_t vec3f_cross(vec3f_t a, vec3f_t b)    { vec3f_t v = {{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }}; return v; } 


vec4f_t vec4f_new(float x, float y, float z, float w) { vec4f_t v = {{ x  , y  , z  , w }}; return v; }
vec4f_t vec4f_from_vec3f(vec3f_t a, float w)          { vec4f_t v = {{ a.x, a.y, a.z, w }}; return v; }
vec4f_t vec4f_min(vec4f_t a, vec4f_t b)      { vec4f_t v = {{ MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z), MIN(a.w, b.w) }}; return v; }
vec4f_t vec4f_max(vec4f_t a, vec4f_t b)      { vec4f_t v = {{ MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z), MAX(a.w, b.w) }}; return v; }
vec4f_t vec4f_add(vec4f_t a, vec4f_t b)      { vec4f_t v = {{    (a.x+ b.x),    (a.y+ b.y),    (a.z+ b.z),    (a.w+ b.w) }}; return v; }
vec4f_t vec4f_sub(vec4f_t a, vec4f_t b)      { vec4f_t v = {{    (a.x- b.x),    (a.y- b.y),    (a.z- b.z),    (a.w- b.w) }}; return v; }
vec4f_t vec4f_mul(vec4f_t a, float   k)      { vec4f_t v = {{    (a.x* k  ),    (a.y* k  ),    (a.z* k  ),    (a.w* k  ) }}; return v; }
float   vec4f_dot(vec4f_t a, vec4f_t b)      { return      (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w); }
float   vec4f_len(vec4f_t a)                 { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w); }
vec4f_t vec4f_normalize(vec4f_t a)           { return vec4f_mul(a, 1 / vec4f_len(a)); }

vec2f_t vec2f_lerp(vec2f_t a, vec2f_t b, float t)
{
    vec2f_t v = {{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
    }};
    return v;
}

vec3f_t vec3f_lerp(vec3f_t a, vec3f_t b, float t)
{
    vec3f_t v = {{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
    }};
    return v;
}

vec4f_t vec4f_lerp(vec4f_t a, vec4f_t b, float t)
{
    vec4f_t v = {{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t,
    }};
    return v;
}

color_t color_lerp(color_t a, color_t b, float t)
{
    color_t c;
    c.argb[0] = a.argb[0] + (b.argb[0] - a.argb[0]) * t;
    c.argb[1] = a.argb[1] + (b.argb[1] - a.argb[1]) * t;
    c.argb[2] = a.argb[2] + (b.argb[2] - a.argb[2]) * t;
    return c;
}
