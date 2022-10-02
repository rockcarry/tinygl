#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "model.h"

typedef struct {
    int num_v, num_vt, num_vn, num_f;
    vec3f_t *lst_v;
    vec3f_t *lst_vt;
    vec3f_t *lst_vn;
    facei_t *lst_f;
    BMP      texture;
} MODEL;

void* model_load(char *object, char *texture)
{
    MODEL *model = NULL;
    FILE  *fp    = fopen(object, "rb");
    int    num_v = 0, num_vt = 0, num_vn = 0, num_f = 0;
    char   buf[256];

    if (!fp) {
        printf("model_load, failed to open file %s !\n", object);
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

    if (bmp_load(&model->texture, texture) != 0) {
        bmp_create(&model->texture, 8, 8, 24);
        bmp_fillrect(&model->texture, 0, 0, 8, 8, RGB(0, 255, 0));
    }
    return model;
}

void model_free(void *ctx)
{
    MODEL *model = (MODEL*)ctx;
    if (model) {
        bmp_destroy(&model->texture);
        free(ctx);
    }
}

void model_save(void *ctx, char *object, char *texture)
{
    MODEL *model = (MODEL*)ctx;
    FILE  *fp    = NULL;
    int    i;
    if (!ctx) return;
    fp = fopen(object, "wb");
    if (!fp) {
        printf("model_save, failed to open file %s !\n", object);
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
    bmp_save(&model->texture, texture);
}

void* model_get_data(void *ctx, int type, int *listsize)
{
    MODEL *model = (MODEL*)ctx;
    if (!ctx) return NULL;
    switch (type) {
    case MODEL_DATA_LIST_V:
        if (listsize) *listsize = model->num_v;
        return model->lst_v;
    case MODEL_DATA_LIST_VT:
        if (listsize) *listsize = model->num_vt;
        return model->lst_vt;
    case MODEL_DATA_LIST_VN:
        if (listsize) *listsize = model->num_vn;
        return model->lst_vn;
    case MODEL_DATA_LIST_F:
        if (listsize) *listsize = model->num_f;
        return model->lst_f;
    case MODEL_DATA_TEXTURE:
        if (listsize) *listsize = 1;
        return &model->texture;
    }
    return NULL;
}

void model_get_face(void *ctx, int idx, facef_t *face)
{
    MODEL *model = (MODEL*)ctx;
    if (ctx) {
        int  i;
        idx %= model->num_f;
        for (i = 0; i < 3; i++) {
            face->v [i] = model->lst_v [(model->lst_f[idx].v [i] - 1) % model->num_v ];
            face->vt[i] = model->lst_vt[(model->lst_f[idx].vt[i] - 1) % model->num_vt];
            face->vn[i] = model->lst_vn[(model->lst_f[idx].vn[i] - 1) % model->num_vn];
        }
    }
}

#ifdef _TEST_MODEL_
#include <float.h>
#include "bmp.h"
#include "matrix.h"
#include "triangle.h"

static void clearzbuffer(BMP *pb, float *zbuf)
{
    if (pb && zbuf) {
        int i, n = pb->width * pb->height;
        for (i = 0; i < n; i++) zbuf[i] = -FLT_MAX;
    }
}

static void projection_matrix(float *m, vec3f_t *camera)
{
    matrix_identity(m , 4);
    m[3 * 4 + 2] = -1.0 / camera->z;
}

static void projection_division(float *m)
{
    m[0 * 1 + 0] = m[0 * 1 + 0] / m[3 * 1 + 0];
    m[1 * 1 + 0] = m[1 * 1 + 0] / m[3 * 1 + 0];
    m[2 * 1 + 0] = m[2 * 1 + 0] / m[3 * 1 + 0];
    m[3 * 1 + 0] = 1.0f;
}

static void viewport_matrix(float *m, int x, int y, int w, int h, int depth)
{
    matrix_identity(m , 4);
    m[0 * 4 + 3] = (float)x + w / 2.0;
    m[1 * 4 + 3] = (float)y + h / 2.0;
    m[2 * 4 + 3] = (float)depth / 2.0;

    m[0 * 4 + 0] = (float)w / 2.0;
    m[1 * 4 + 1] = (float)h /-2.0;
    m[2 * 4 + 2] = (float)depth / 2.0;
}

static void world2screen_old(BMP *pb, vec3f_t *screen, vec3f_t *world)
{
    screen->x = 0.5 * (world->x + 1) * (pb->width - 1);
    screen->y = (pb->height - 1) - 0.5 * (world->y + 1) * (pb->height - 1);
    screen->z = world->z;
}

static void world2screen(vec3f_t *screen, vec3f_t *world, float *mat_model, float *mat_view, float *mat_proj, float *mat_port)
{
    float mat_tmp1[4] = { world->x, world->y, world->z, 1.0 };
    float mat_tmp2[4];
    float mat_tmp3[4 * 4], mat_mvp[4 * 4];

    matrix_mul(mat_tmp3, mat_proj , 4, 4, mat_view , 4, 4);
    matrix_mul(mat_mvp , mat_tmp3 , 4, 4, mat_model, 4, 4);
    matrix_mul(mat_tmp2, mat_mvp  , 4, 4, mat_tmp1 , 4, 1);

    projection_division(mat_tmp2);
    matrix_mul(mat_tmp1, mat_port , 4, 4, mat_tmp2, 4, 1);

    screen->x = mat_tmp1[0];
    screen->y = mat_tmp1[1];
    screen->z = mat_tmp1[2];
}

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 1024

int main(void)
{
    BMP     mybmp = {}, *texture = NULL;
    void   *model = model_load("head.obj", "head.bmp");
    int     nface = 0, i, j;
    facef_t fface = {};
    vec3f_t light = { .z = -1 };
    vec3f_t camera= { .z =  3 };
    vec3f_t norm  = {};
    vec3f_t tpts[3];
    vec3f_t vec1, vec2;
    float   intensity = 0, *zbuf = NULL;

    float matrix_model[4 * 4];
    float matrix_view [4 * 4];
    float matrix_proj [4 * 4];
    float matrix_port [4 * 4];

    bmp_create(&mybmp, 1024, 1024, 24);
    model_get_data(model, MODEL_DATA_LIST_F, &nface);
    texture = model_get_data(model, MODEL_DATA_TEXTURE, NULL);

    zbuf = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(float));
    clearzbuffer(&mybmp, zbuf);

    matrix_identity(matrix_model, 4);
    matrix_identity(matrix_view , 4);
    projection_matrix(matrix_proj, &camera);
    viewport_matrix  (matrix_port, mybmp.width / 8, mybmp.height / 8, mybmp.width * 3 / 4, mybmp.height * 3 / 4, 255);

    for (i = 0; i < nface; i++) {
        model_get_face(model, i, &fface);
        for (j = 0; j < 3; j++) world2screen(tpts + j, fface.v + j, matrix_model, matrix_view, matrix_proj, matrix_port);
        vector3f_sub((float*)&vec1, (float*)(fface.v + 2), (float*)(fface.v + 0));
        vector3f_sub((float*)&vec2, (float*)(fface.v + 1), (float*)(fface.v + 0));
        vector3f_cross((float*)&norm, (float*)&vec1, (float*)&vec2);
        vector3f_norm ((float*)&norm);
        intensity = vector3f_dot((float*)&norm, (float*)&light);
        if (intensity > 0) triangle(&mybmp, texture, zbuf, tpts, fface.vt, intensity, RGB(255 * intensity, 255 * intensity, 255 * intensity));
    }

    free(zbuf);
    bmp_save(&mybmp, "out.bmp");
    bmp_destroy(&mybmp);
    model_free(model);
    return 0;
}
#endif
