#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "vector.h"

#define matrix_add(mr, ma, mb, row, col) vectornf_add(mr, ma, mb, row * col)
#define matrix_sub(mr, ma, mb, row, col) vectornf_sub(mr, ma, mb, row * col)

void matrix_mul(float *mr, float *ma, int rowa, int cola, float *mb, int rowb, int colb);
void matrix_transpose(float *mr, int rowr, int colr, float *ma, int rowa, int cola);
void matrix_identity (float *m, int n);

#endif
