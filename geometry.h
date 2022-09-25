#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

void  vectorni_add  (int *r, int *a, int *b, int n);
void  vectorni_sub  (int *r, int *a, int *b, int n);
void  vectorni_mul  (int *r, int  k, int *a, int n);
void  vector3i_cross(int *r, int *a, int *b);
int   vectorni_dot  (int *a, int *b, int  n);
int   vectorni_abs  (int *a, int n);

void  vectornf_add  (float *r, float *a, float *b, int n);
void  vectornf_sub  (float *r, float *a, float *b, int n);
void  vectornf_mul  (float *r, float  k, float *a, int n);
void  vector3f_cross(float *r, float *a, float *b);
float vectornf_dot  (float *a, float *b, int n);
float vectornf_abs  (float *a, int n);

#endif
