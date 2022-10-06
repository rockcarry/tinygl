#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "matrix.h"
#include "model.h"
#include "tinygl.h"
#include "wingdi.h"
#include "utils.h"

static char *s_model_list[][2] = {
    { "model/head.obj"          , "model/head.tga"           }, // 人头
    { "model/head_eye_inner.obj", "model/head_eye_inner.tga" }, // 眼睛
};

static char *s_shader_list[][2] = {
    { "wire"   , "none"    }, // 线框
    { "rand"   , "color0"  }, // 随机颜色，色块填充
    { "rand"   , "color1"  }, // 随机颜色，渐变填充
    { "flat"   , "color0"  }, // flat 着色，颜色填充
    { "gouraud", "color1"  }, // gouraud 着色，颜色填充
    { "mvpp"   , "phong"   }, // phong 着色，颜色填充
    { "flat"   , "normal0" }, // flat 向量着色
    { "mvpp"   , "normal1" }, // phong 向量着色
    { "flat"   , "texture1"}, // flat 纹理映射
    { "mvpp"   , "texture0"}, // 直接纹理映射
    { "mvpp"   , "texture2"}, // phong 纹理映射
};

static void my_winmsg_callback(void *cbctx, int msg, uint32_t param1, uint32_t param2, void *param3)
{
    int *change = (int*)cbctx;
    switch (msg) {
    case WINGDI_MSG_KEY_EVENT:
        printf("key: %d, %s\n", param2, param1 ? "pressed" : "released"); fflush(stdout);
        if (param1 && param2 == ' ') *change = 1;
        break;
    }
}

int main(void)
{
    void *models[ARRAYSIZE(s_model_list)] = { NULL };
    int   angle = 0, curshader = 0, change = 1, i;
    void *win = wingdi_init(640, 640, my_winmsg_callback, &change);
    void *gl  = tinygl_init(0  , 0  );
    uint32_t fratectrl[4];

    tinygl_set(gl, "target", wingdi_get(win, "texture"));
    tinygl_set(gl, "shader.target", wingdi_get(win, "texture"));

    for (i = 0; i < ARRAYSIZE(models); i++) models[i] = model_load(s_model_list[i][0], s_model_list[i][1]);
    while (strcmp(wingdi_get(win, "state"), "closed") != 0) {
        if (change) {
            tinygl_set(gl, "shader.vertex", s_shader_list[curshader][0]);
            tinygl_set(gl, "shader.fragmt", s_shader_list[curshader][1]);
            change = 0, curshader = (curshader + 1) % ARRAYSIZE(s_shader_list);
        }
        mat4f_t matrot = mat4f_rotate_y(angle * 2 * M_PI / 360); angle += 2;
        tinygl_set(gl, "shader.mat_model", &matrot);
        tinygl_begin(gl, 1);
        for (i = 0; i < ARRAYSIZE(models); i++) {
            tinygl_set (gl, "shader.texture", model_get_texture(models[i]));
            tinygl_draw(gl, models[i]);
        }
        tinygl_end(gl);
        frame_rate_control(fratectrl, 60);
    }
    for (i = 0; i < ARRAYSIZE(models); i++) { model_free(models[i]); models[i] = NULL; }

    tinygl_free(gl);
    wingdi_free(win, 0);
    return 0;
}
