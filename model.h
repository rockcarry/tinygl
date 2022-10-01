#ifndef __MODEL_H__
#define __MODEL_H__

#include "geometry.h"

enum {
    MODEL_DATA_LIST_V,
    MODEL_DATA_LIST_VT,
    MODEL_DATA_LIST_VN,
    MODEL_DATA_LIST_F,
    MODEL_DATA_TRIANGLE,
};

typedef struct {
    int v[3], vt[3], vn[3];
} facei_t;

typedef struct {
    vec3f_t v[3], vt[3], vn[3];
} facef_t;

void* model_load(char *file);
void  model_save(void *ctx, char *file);
void  model_free(void *ctx );
void  model_get (void *ctx, int type, int i, void *data, int *n);

#endif
