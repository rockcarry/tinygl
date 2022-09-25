#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "model.h"

typedef struct {
    int num_v, num_vt, num_vn, num_f;
    float *buf_v;
    float *buf_vt;
    float *buf_vn;
    int   *buf_f;
} MODEL;

void* model_load(char *file)
{
    MODEL *model = NULL;
    FILE  *fp    = fopen(file, "rb");
    int    num_v = 0, num_vt = 0, num_vn = 0, num_f = 0;
    char   buf[256];
    if (!fp) {
        printf("model_load, failed to open file %s !\n", file);
        return NULL;
    }

    while (!feof(fp)) {
        fscanf(fp, "%255s", buf);
        if      (strcmp(buf, "v" ) == 0) num_v ++;
        else if (strcmp(buf, "vt") == 0) num_vt++;
        else if (strcmp(buf, "vn") == 0) num_vn++;
        else if (strcmp(buf, "f" ) == 0) num_f ++;
        fgets(buf, sizeof(buf), fp);
    }

    model = malloc(sizeof(MODEL) + (num_v + num_vt + num_vn) * 3 * sizeof(float) + num_f * 9 * sizeof(int));
    if (model) {
        memset(model, 0, sizeof(MODEL));
        model->buf_v  = (float*)((char*)model + sizeof(MODEL));
        model->buf_vt = (float*)(model->buf_v  + num_v  * 3);
        model->buf_vn = (float*)(model->buf_vt + num_vt * 3);
        model->buf_f  = (int  *)(model->buf_vn + num_vn * 3);
        fseek(fp, 0, SEEK_SET);
        while (!feof(fp)) {
            fscanf(fp, "%255s", buf);
            if (strcmp(buf, "v" ) == 0 && model->num_v < num_v) {
                fscanf(fp, "%f %f %f", model->buf_v + model->num_v * 3 + 0, model->buf_v + model->num_v * 3 + 1, model->buf_v + model->num_v * 3 + 2);
                model->num_v++;
            } else if (strcmp(buf, "vt") == 0 && model->num_vt < num_vt) {
                fscanf(fp, "%f %f %f", model->buf_vt+ model->num_vt* 3 + 0, model->buf_vt+ model->num_vt* 3 + 1, model->buf_vt+ model->num_vt* 3 + 2);
                model->num_vt++;
            } else if (strcmp(buf, "vn") == 0 && model->num_vn < num_vn) {
                fscanf(fp, "%f %f %f", model->buf_vn+ model->num_vn* 3 + 0, model->buf_vn+ model->num_vn* 3 + 1, model->buf_vn+ model->num_vn* 3 + 2);
                model->num_vn++;
            } else if (strcmp(buf, "f" ) == 0 && model->num_f  < num_f ) {
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    model->buf_f+ model->num_f * 9 + 0, model->buf_f+ model->num_f * 9 + 1, model->buf_f+ model->num_f * 9 + 2,
                    model->buf_f+ model->num_f * 9 + 3, model->buf_f+ model->num_f * 9 + 4, model->buf_f+ model->num_f * 9 + 5,
                    model->buf_f+ model->num_f * 9 + 6, model->buf_f+ model->num_f * 9 + 7, model->buf_f+ model->num_f * 9 + 8);
                model->num_f++;
            }
        }
    }

    fclose(fp);
    return model;
}

void model_free(void *ctx)
{
    free(ctx);
}

void model_save(void *ctx, char *file)
{
    MODEL *model = (MODEL*)ctx;
    FILE  *fp    = NULL;
    int    i;
    if (!ctx) return;
    fp = fopen(file, "wb");
    if (!fp) {
        printf("model_save, failed to open file %s !\n", file);
        return;
    }
    for (i = 0; i < model->num_v; i++) {
        fprintf(fp, "v  %f %f %f\n", model->buf_v [i * 3 + 0], model->buf_v [i * 3 + 1], model->buf_v [i * 3 + 2]);
    }
    for (i = 0; i < model->num_vt; i++) {
        fprintf(fp, "vt %f %f %f\n", model->buf_vt[i * 3 + 0], model->buf_vt[i * 3 + 1], model->buf_vt[i * 3 + 2]);
    }
    for (i = 0; i < model->num_vn; i++) {
        fprintf(fp, "vn %f %f %f\n", model->buf_vn[i * 3 + 0], model->buf_vn[i * 3 + 1], model->buf_vn[i * 3 + 2]);
    }
    for (i = 0; i < model->num_f; i++) {
        fprintf(fp, "f  %d/%d/%d %d/%d/%d %d/%d/%d\n",
            model->buf_f[i * 9 + 0], model->buf_f[i * 9 + 1], model->buf_f[i * 9 + 2],
            model->buf_f[i * 9 + 3], model->buf_f[i * 9 + 4], model->buf_f[i * 9 + 5],
            model->buf_f[i * 9 + 6], model->buf_f[i * 9 + 7], model->buf_f[i * 9 + 8]);
    }
    fclose(fp);
}

void model_get(void *ctx, int type, int i, void *data, int *n)
{
    MODEL *model = (MODEL*)ctx;
    int    j, k;
    if (!ctx) return;
    switch (type) {
    case MODEL_DATA_LIST_V:
        if (data) *(float**)data = model->buf_v;
        if (n   ) *(int   *)n    = model->num_v;
        break;
    case MODEL_DATA_LIST_VT:
        if (data) *(float**)data = model->buf_vt;
        if (n   ) *(int   *)n    = model->num_vt;
        break;
    case MODEL_DATA_LIST_VN:
        if (data) *(float**)data = model->buf_vn;
        if (n   ) *(int   *)n    = model->num_vn;
        break;
    case MODEL_DATA_LIST_F:
        if (data) *(int  **)data = model->buf_f;
        if (n   ) *(int   *)n    = model->num_f;
        break;
    case MODEL_DATA_TRIANGLE:
        i %= model->num_f;
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                ((float*)data)[j * 9 + 0 * 3 + k] = model->buf_v [(model->buf_f[i * 9 + j * 3 + 0] - 1) % model->num_v  * 3 + k];
                ((float*)data)[j * 9 + 1 * 3 + k] = model->buf_vt[(model->buf_f[i * 9 + j * 3 + 1] - 1) % model->num_vt * 3 + k];
                ((float*)data)[j * 9 + 2 * 3 + k] = model->buf_vn[(model->buf_f[i * 9 + j * 3 + 2] - 1) % model->num_vn * 3 + k];
            }
        }
        break;
    }
}

#ifdef _TEST_
#include "bmp.h"
int main(void)
{
    BMP    mybmp = {};
    void  *model = model_load("african_head.obj");
    int    f_num = 0, i, j;
    float  triangle[27];
    bmp_create(&mybmp, 1027, 768, 24);
    model_get (model, MODEL_DATA_LIST_F, 0, NULL, &f_num);
    for (i = 0; i < f_num; i++) {
        model_get(model, MODEL_DATA_TRIANGLE, i, &triangle, NULL);
        for (j = 0; j < 3; j++) {
            float fx1 = triangle[(j + 0) % 3 * 9 + 0];
            float fy1 = triangle[(j + 0) % 3 * 9 + 1];
            float fx2 = triangle[(j + 1) % 3 * 9 + 0];
            float fy2 = triangle[(j + 1) % 3 * 9 + 1];
            int   ix1 = (fx1 + 1) * mybmp.width  / 2;
            int   iy1 = mybmp.height - 1 - (fy1 + 1) * mybmp.height / 2;
            int   ix2 = (fx2 + 1) * mybmp.width  / 2;
            int   iy2 = mybmp.height - 1 - (fy2 + 1) * mybmp.height / 2;
            bmp_line(&mybmp, ix1, iy1, ix2, iy2, RGB(0, 255, 0));
        }
    }
    bmp_save(&mybmp, "out.bmp");
    model_free(model);
    bmp_destroy(&mybmp);
    return 0;
}
#endif
