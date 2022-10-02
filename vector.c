#include <math.h>
#include "vector.h"

#define VECTOR_ADD(r, a, b, n) \
do { \
    int  i; \
    for (i = 0; i < n; i++) { \
        (r)[i] = (a)[i] + (b)[i]; \
    } \
} while (0)

#define VECTOR_SUB(r, a, b, n) \
do { \
    int  i; \
    for (i = 0; i < n; i++) { \
        (r)[i] = (a)[i] - (b)[i]; \
    } \
} while (0)

#define VECTOR_MUL(r, a, k, n) \
do { \
    int  i; \
    for (i = 0; i < n; i++) { \
        (r)[i] = (a)[i] * k; \
    } \
} while (0)

#define VECTOR3_CROSS(r, a, b) \
do { \
    (r)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
    (r)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
    (r)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]; \
} while (0)

#define VECTOR_DOT(r, a, b, n) \
do { \
    int  i; \
    for (i = 0; i < n; i++) { \
        (r) += (a)[i] * (b)[i]; \
    } \
} while (0)

#define VECTOR_ABS(r, a, n) \
do { \
    int  i; \
    for (i = 0; i < n; i++) { \
        (r) += (a)[i] * (a)[i]; \
    } \
    r = sqrtf(r); \
} while (0)

void vectorni_add  (int *r, int *a, int *b, int n) { VECTOR_ADD(r, a, b, n); }
void vectorni_sub  (int *r, int *a, int *b, int n) { VECTOR_SUB(r, a, b, n); }
void vectorni_mul  (int *r, int *a, int  k, int n) { VECTOR_MUL(r, a, k, n); }
void vector3i_cross(int *r, int *a, int *b)        { VECTOR3_CROSS(r, a, b); }

int vectorni_dot(int *a, int *b, int n)
{
    int r = 0;
    VECTOR_DOT(r, a, b, n);
    return r;
}

int vectorni_abs(int *a, int n)
{
    int r = 0;
    VECTOR_ABS(r, a, n);
    return r;
}

void vectornf_add  (float *r, float *a, float *b, int n) { VECTOR_ADD(r, a, b, n); }
void vectornf_sub  (float *r, float *a, float *b, int n) { VECTOR_SUB(r, a, b, n); }
void vectornf_mul  (float *r, float *a, float  k, int n) { VECTOR_MUL(r, a, k, n); }
void vector3f_cross(float *r, float *a, float *b)        { VECTOR3_CROSS(r, a, b); }

float vectornf_dot(float *a, float *b, int n)
{
    float r = 0;
    VECTOR_DOT(r, a, b, n);
    return r;
}

float vectornf_abs(float *a, int n)
{
    float r = 0;
    VECTOR_ABS(r, a, n);
    return r;
}

void vectornf_norm(float *a, int n)
{
    float abs = 1 / vectornf_abs(a, n);
    int   i;
    for (i = 0; i < n; i++) a[i] *= abs;
}
