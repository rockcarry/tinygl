#ifndef __MODEL_H__
#define __MODEL_H__

#include "vector.h"

void* model_load(char *object, char *texture);
void  model_save(void *ctx, char *object, char *texture);
void  model_free(void *ctx );
void* model_get_texture(void *ctx);
int   model_get_face   (void *ctx, int idx, vertex_t face[3]); // idx == -1 query the face number

#endif
