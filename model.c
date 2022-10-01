#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "model.h"

typedef struct {
    int num_v, num_vt, num_vn, num_f;
    vec3f_t *lst_v;
    vec3f_t *lst_vt;
    vec3f_t *lst_vn;
    facei_t *lst_f;
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
        model->lst_v  = (vec3f_t*)((char*)model + sizeof(MODEL));
        model->lst_vt = (vec3f_t*)(model->lst_v  + num_v );
        model->lst_vn = (vec3f_t*)(model->lst_vt + num_vt);
        model->lst_f  = (facei_t*)(model->lst_vn + num_vn);
        fseek(fp, 0, SEEK_SET);
        while (!feof(fp)) {
            fscanf(fp, "%255s", buf);
            if (strcmp(buf, "v" ) == 0 && model->num_v < num_v) {
                fscanf(fp, "%f %f %f", &(model->lst_v[model->num_v].x), &(model->lst_v[model->num_v].y), &(model->lst_v[model->num_v].z));
                model->num_v++;
            } else if (strcmp(buf, "vt") == 0 && model->num_vt < num_vt) {
                fscanf(fp, "%f %f %f", &(model->lst_vt[model->num_vt].x), &(model->lst_vt[model->num_vt].y), &(model->lst_vt[model->num_vt].z));
                model->num_vt++;
            } else if (strcmp(buf, "vn") == 0 && model->num_vn < num_vn) {
                fscanf(fp, "%f %f %f", &(model->lst_vn[model->num_vn].x), &(model->lst_vn[model->num_vn].y), &(model->lst_vn[model->num_vn].z));
                model->num_vn++;
            } else if (strcmp(buf, "f" ) == 0 && model->num_f  < num_f ) {
                fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d",
                    &(model->lst_f[model->num_f].v[0]), &(model->lst_f[model->num_f].vt[0]), &(model->lst_f[model->num_f].vn[0]),
                    &(model->lst_f[model->num_f].v[1]), &(model->lst_f[model->num_f].vt[1]), &(model->lst_f[model->num_f].vn[1]),
                    &(model->lst_f[model->num_f].v[2]), &(model->lst_f[model->num_f].vt[2]), &(model->lst_f[model->num_f].vn[2]));
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
        fprintf(fp, "v  %f %f %f\n", model->lst_v[i].x, model->lst_v[i].y, model->lst_v[i].z);
    }
    for (i = 0; i < model->num_vt; i++) {
        fprintf(fp, "vt %f %f %f\n", model->lst_vt[i].x, model->lst_vt[i].y, model->lst_vt[i].z);
    }
    for (i = 0; i < model->num_vn; i++) {
        fprintf(fp, "vn %f %f %f\n", model->lst_vn[i].x, model->lst_vn[i].y, model->lst_vn[i].z);
    }
    for (i = 0; i < model->num_f; i++) {
        fprintf(fp, "f  %d/%d/%d %d/%d/%d %d/%d/%d\n",
            model->lst_f[i].v[0], model->lst_f[i].vt[0], model->lst_f[i].vn[0],
            model->lst_f[i].v[1], model->lst_f[i].vt[1], model->lst_f[i].vn[1],
            model->lst_f[i].v[2], model->lst_f[i].vt[2], model->lst_f[i].vn[2]);
    }
    fclose(fp);
}

void model_get(void *ctx, int type, int i, void *data, int *n)
{
    MODEL *model = (MODEL*)ctx;
    int    j;
    if (!ctx) return;
    switch (type) {
    case MODEL_DATA_LIST_V:
        if (data) *(vec3f_t**)data = model->lst_v;
        if (n   ) *(int     *)n    = model->num_v;
        break;
    case MODEL_DATA_LIST_VT:
        if (data) *(vec3f_t**)data = model->lst_vt;
        if (n   ) *(int     *)n    = model->num_vt;
        break;
    case MODEL_DATA_LIST_VN:
        if (data) *(vec3f_t**)data = model->lst_vn;
        if (n   ) *(int     *)n    = model->num_vn;
        break;
    case MODEL_DATA_LIST_F:
        if (data) *(facei_t**)data = model->lst_f;
        if (n   ) *(int     *)n    = model->num_f;
        break;
    case MODEL_DATA_TRIANGLE:
        i %= model->num_f;
        for (j = 0; j < 3; j++) {
            ((facef_t*)data)->v [j] = model->lst_v [(model->lst_f[i].v [j] - 1) % model->num_v ];
            ((facef_t*)data)->vt[j] = model->lst_vt[(model->lst_f[i].vt[j] - 1) % model->num_vt];
            ((facef_t*)data)->vn[j] = model->lst_vn[(model->lst_f[i].vn[j] - 1) % model->num_vn];
        }
        break;
    }
}

#ifdef _TEST_MODEL_
#include <float.h>
#include "bmp.h"
#include "geometry.h"
#include "triangle.h"

static void world2screen(BMP *pb, vec3f_t *screen, vec3f_t *world)
{
    screen->x = 0.5 * (world->x + 1) * (pb->width - 1);
    screen->y = (pb->height - 1) - 0.5 * (world->y + 1) * (pb->height - 1);
    screen->z = world->z;
}

static void clearzbuffer(BMP *pb, float *zbuf)
{
    if (pb && zbuf) {
        int i, n = pb->width * pb->height;
        for (i = 0; i < n; i++) zbuf[i] = -FLT_MAX;
    }
}

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

int main(void)
{
    BMP     mybmp = {};
    void   *model = model_load("head.obj");
    int     nface = 0, i, j;
    facef_t fface = {};
    vec3f_t light = { .z = -1 };
    vec3f_t norm  = {};
    vec3f_t tpts[3];
    vec3f_t vec1, vec2;
    float   intensity = 0, *zbuf = NULL;

    bmp_create(&mybmp, 1024, 1024, 24);
    model_get (model, MODEL_DATA_LIST_F, 0, NULL, &nface);

    zbuf = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(float));
    clearzbuffer(&mybmp, zbuf);

    for (i = 0; i < nface; i++) {
        model_get(model, MODEL_DATA_TRIANGLE, i, &fface, NULL);
        for (j = 0; j < 3; j++) world2screen(&mybmp, tpts + j, fface.v + j);
        vector3f_sub((float*)&vec1, (float*)(fface.v + 2), (float*)(fface.v + 0));
        vector3f_sub((float*)&vec2, (float*)(fface.v + 1), (float*)(fface.v + 0));
        vector3f_cross((float*)&norm, (float*)&vec1, (float*)&vec2);
        vector3f_norm ((float*)&norm);
        intensity = vector3f_dot((float*)&norm, (float*)&light);
//      triangle(&mybmp, zbuf, tpts, RGB(0, 255, 0), 0);
//      triangle(&mybmp, zbuf, tpts, RGB(rand(), rand(), rand()), 1);
        if (intensity > 0) triangle(&mybmp, zbuf, tpts, RGB(255 * intensity, 255 * intensity, 255 * intensity), 1);
    }

    bmp_save(&mybmp, "out.bmp");
    free(zbuf);
    bmp_destroy(&mybmp);
    model_free(model);
    return 0;
}
#endif
