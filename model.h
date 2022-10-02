#ifndef __MODEL_H__
#define __MODEL_H__

#include "vector.h"

enum {
    MODEL_DATA_LIST_V,
    MODEL_DATA_LIST_VT,
    MODEL_DATA_LIST_VN,
    MODEL_DATA_LIST_F,
    MODEL_DATA_TEXTURE,
};

typedef struct {
    int v[3], vt[3], vn[3];
} facei_t;

void* model_load(char *object, char *texture);
void  model_save(void *ctx, char *object, char *texture);
void  model_free(void *ctx );
void* model_get_data(void *ctx, int type, int *listsize);
void  model_get_face(void *ctx, int idx, vertex_t face[3]);

#endif
