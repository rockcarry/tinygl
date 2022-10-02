#ifndef __VECTOR_H__
#define __VECTOR_H__

typedef struct {
    union {
        struct { int x, y; };
        struct { int u, v; };
        int raw[2];
    };
} vec2i_t;

typedef struct {
    union {
        struct { int x, y, z; };
        struct { int u, v; };
        int raw[3];
    };
} vec3i_t;

typedef struct {
    union {
        struct { float x, y; };
        struct { float u, v; };
        float raw[2];
    };
} vec2f_t;

typedef struct {
    union {
        struct { float x, y, z; };
        struct { float u, v; };
        float raw[3];
    };
} vec3f_t;

void  vectorni_add  (int *r, int *a, int *b, int n);
void  vectorni_sub  (int *r, int *a, int *b, int n);
void  vectorni_mul  (int *r, int *a, int  k, int n);
void  vector3i_cross(int *r, int *a, int *b);
int   vectorni_dot  (int *a, int *b, int  n);
int   vectorni_abs  (int *a, int n);

void  vectornf_add  (float *r, float *a, float *b, int n);
void  vectornf_sub  (float *r, float *a, float *b, int n);
void  vectornf_mul  (float *r, float *a, float  k, int n);
void  vector3f_cross(float *r, float *a, float *b);
float vectornf_dot  (float *a, float *b, int n);
float vectornf_abs  (float *a, int n);
void  vectornf_norm (float *a, int n);

#define vector3i_add(r, a, b)   vectorni_add(r, a, b, 3)
#define vector3i_sub(r, a, b)   vectorni_sub(r, a, b, 3)
#define vector3i_mul(r, a, k)   vectorni_add(r, a, k, 3)
#define vector3i_dot(a, b)      vectorni_dot(a, b, 3)
#define vector3i_abs(a)         vectorni_abs(a, 3)

#define vector3f_add(r, a, b)   vectornf_add(r, a, b, 3)
#define vector3f_sub(r, a, b)   vectornf_sub(r, a, b, 3)
#define vector3f_mul(r, a, k)   vectornf_add(r, a, k, 3)
#define vector3f_dot(a, b)      vectornf_dot(a, b, 3)
#define vector3f_abs(a)         vectornf_abs(a, 3)
#define vector3f_norm(a)        vectornf_norm(a, 3)

#endif
