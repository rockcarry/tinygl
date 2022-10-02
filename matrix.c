#include <stdlib.h>
#include <stdio.h>
#include "vector.h"

void matrix_mul(float *mr, float *ma, int rowa, int cola, float *mb, int rowb, int colb)
{
    int i, j, k;
    if (cola != rowb) {
        printf("incorrect matrix size, rowa: %d, cola: %d, rowb: %d, colb: %d !\n", rowa, cola, rowb, colb);
        return;
    }
    for (i = 0; i < rowa; i++) {
        for (j = 0; j < colb; j++) {
            float sum = 0;
            for (k = 0; k < cola; k++) {
                sum += ma[i * cola + k] * mb[k * colb + j];
            }
            mr[i * colb + j] = sum;
        }
    }
}

void matrix_transpose(float *mr, int rowr, int colr, float *ma, int rowa, int cola)
{
    int i, j;
    if (rowr != cola || colr != rowa) {
        printf("incorrect matrix size, rowr: %d, colr: %d, rowa: %d, cola: %d !\n", rowr, colr, rowa, cola);
        return;
    }
    for (i = 0; i < rowr; i++) {
        for (j = 0; j < colr; j++) {
            mr[i * colr + j] = ma[j * cola + i];
        }
    }
}

void matrix_identity(float *m, int n)
{
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            m[i * n + j] = (i == j) ? 1 : 0;
        }
    }
}
