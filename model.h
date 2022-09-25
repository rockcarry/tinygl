#ifndef __MODEL_H__
#define __MODEL_H__

void* model_load(char *file);
void  model_save(void *ctx, char *file);
void  model_free(void *ctx );

enum {
    MODEL_DATA_LIST_V,
    MODEL_DATA_LIST_VT,
    MODEL_DATA_LIST_VN,
    MODEL_DATA_LIST_F,
    MODEL_DATA_TRIANGLE,
};
void model_get(void *ctx, int type, int i, void *data, int *n);

#endif
