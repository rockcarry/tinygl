#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "model.h"

typedef struct {
    int num_v, num_vt, num_vn, num_f;
    vec3f_t *lst_v;
    vec3f_t *lst_vt;
    vec3f_t *lst_vn;
    facei_t *lst_f;
    TEXTURE *texture;
} MODEL;

void* model_load(char *fileobj, char *filetext)
{
    MODEL *model = NULL;
    FILE  *fp    = fopen(fileobj, "rb");
    int    num_v = 0, num_vt = 0, num_vn = 0, num_f = 0;
    char   buf[256];

    if (!fp) {
        printf("model_load, failed to open file %s !\n", fileobj);
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
    if (!model) goto done;

    memset(model, 0, sizeof(MODEL));
    model->lst_v  = (vec3f_t*)(model + 1);
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
    model->texture = texture_load(filetext);
    if (model->texture == NULL) {
        model->texture = texture_create(8, 8, 24);
        texture_fillrect(model->texture, 0, 0, 8, 8, RGB(0, 255, 0));
    }

done:
    fclose(fp);
    return model;
}

void model_free(void *ctx)
{
    MODEL *model = (MODEL*)ctx;
    if (model) {
        texture_destroy(model->texture);
        free(ctx);
    }
}

void model_save(void *ctx, char *fileobj, char *filetext)
{
    MODEL *model = (MODEL*)ctx;
    FILE  *fp;
    int    i;
    if (!ctx) return;
    fp = fopen(fileobj, "wb");
    if (!fp) { printf("model_save, failed to open file %s !\n", fileobj); return; }
    for (i = 0; i < model->num_v ; i++) fprintf(fp, "v  %f %f %f\n", model->lst_v [i].x, model->lst_v [i].y, model->lst_v [i].z);
    for (i = 0; i < model->num_vt; i++) fprintf(fp, "vt %f %f %f\n", model->lst_vt[i].x, model->lst_vt[i].y, model->lst_vt[i].z);
    for (i = 0; i < model->num_vn; i++) fprintf(fp, "vn %f %f %f\n", model->lst_vn[i].x, model->lst_vn[i].y, model->lst_vn[i].z);
    for (i = 0; i < model->num_f; i++) {
        fprintf(fp, "f  %d/%d/%d %d/%d/%d %d/%d/%d\n",
            model->lst_f[i].v[0], model->lst_f[i].vt[0], model->lst_f[i].vn[0],
            model->lst_f[i].v[1], model->lst_f[i].vt[1], model->lst_f[i].vn[1],
            model->lst_f[i].v[2], model->lst_f[i].vt[2], model->lst_f[i].vn[2]);
    }
    fclose(fp);
    texture_save(model->texture, filetext);
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

void model_get_face(void *ctx, int idx, vertex_t face[3])
{
    MODEL *model = (MODEL*)ctx;
    if (ctx) {
        int  i;
        idx %= model->num_f;
        for (i = 0; i < 3; i++) {
            face[i].v .x     = model->lst_v [(model->lst_f[idx].v [i] - 1) % model->num_v ].x;
            face[i].v .y     = model->lst_v [(model->lst_f[idx].v [i] - 1) % model->num_v ].y;
            face[i].v .z     = model->lst_v [(model->lst_f[idx].v [i] - 1) % model->num_v ].z;
            face[i].v .w     = 1;
            face[i].vt.u     = model->lst_vt[(model->lst_f[idx].vt[i] - 1) % model->num_vt].u;
            face[i].vt.v     = model->lst_vt[(model->lst_f[idx].vt[i] - 1) % model->num_vt].v;
            face[i].vn.alpha = model->lst_vn[(model->lst_f[idx].vn[i] - 1) % model->num_vn].alpha;
            face[i].vn.beta  = model->lst_vn[(model->lst_f[idx].vn[i] - 1) % model->num_vn].beta;
            face[i].vn.gamma = model->lst_vn[(model->lst_f[idx].vn[i] - 1) % model->num_vn].gamma;
        }
    }
}

#ifdef _TEST_MODEL_
int main(void)
{
    void    *model    = model_load("head.obj", "head.bmp");
    int      nface    = 0, i;
    vertex_t triangle[3];

    model_get_data(model, MODEL_DATA_LIST_F, &nface);
    for (i = 0; i < nface; i++) {
        model_get_face(model, i, triangle);
    }

    model_save(model, "out.obj", NULL);
    model_free(model);
    return 0;
}
#endif
