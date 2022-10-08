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
    { "model/head.obj"          , "model/head.tga"           }, // ��ͷ
    { "model/head_eye_inner.obj", "model/head_eye_inner.tga" }, // �۾�
};

static char *s_shader_list[][2] = {
    { "wire"   , "none"    }, // �߿�
    { "rand"   , "color0"  }, // �����ɫ��ɫ�����
    { "rand"   , "color1"  }, // �����ɫ���������
    { "flat"   , "color0"  }, // flat ��ɫ����ɫ���
    { "gouraud", "color1"  }, // gouraud ��ɫ����ɫ���
    { "mvpp"   , "phong"   }, // phong ��ɫ����ɫ���
    { "flat"   , "normal0" }, // flat ������ɫ
    { "mvpp"   , "normal1" }, // phong ������ɫ
    { "flat"   , "texture1"}, // flat ����ӳ��
    { "mvpp"   , "texture0"}, // ֱ������ӳ��
    { "mvpp"   , "texture2"}, // phong ����ӳ��
};

typedef struct {
    int      change_shader;
    vec3f_t  camera_position;
    vec3f_t  camera_target;
    vec3f_t  camera_up;
} DEMO;

static void my_winmsg_callback(void *cbctx, int msg, uint32_t param1, uint32_t param2, void *param3)
{
    DEMO *demo = (DEMO*)cbctx;
    switch (msg) {
    case WINGDI_MSG_KEY_EVENT:
        printf("key: %d, %s\n", param2, param1 ? "pressed" : "released"); fflush(stdout);
        if (param1) {
            switch (param2) {
            case ' ': demo->change_shader = 1; break;
            case 'E': case 'e': demo->camera_position.z -= 0.5; demo->camera_target.y = demo->camera_position.y; break;
            case 'D': case 'd': demo->camera_position.z += 0.5; demo->camera_target.y = demo->camera_position.y; break;
            case 'S': case 's': demo->camera_position.x -= 0.1; demo->camera_target.x = demo->camera_position.x; break;
            case 'F': case 'f': demo->camera_position.x += 0.1; demo->camera_target.x = demo->camera_position.x; break;
            }
        }
        break;
    }
}

int main(void)
{
    DEMO  demo= { 1, {{ 0, 0, 3 }}, {{ 0, 0, -1 }}, {{ 0, 1, 0 }} };
    void *models[ARRAYSIZE(s_model_list)] = { NULL };
    int   curshader = 3, angle = 0, i;
    void *win = wingdi_init(640, 480, my_winmsg_callback, &demo);
    void *gl  = tinygl_init(0, 0);
    mat4f_t matmodel, matview, matproj;
    uint32_t frc[4];

    tinygl_set(gl, "target"       , wingdi_get(win, "texture"));
    tinygl_set(gl, "shader.target", wingdi_get(win, "texture"));
    matproj = mat4f_perspective(60 * 2 * M_PI / 360, 640.0 / 480.0, 0.1, 10000);
    tinygl_set(gl, "shader.mat_proj", &matproj );

    for (i = 0; i < ARRAYSIZE(models); i++) models[i] = model_load(s_model_list[i][0], s_model_list[i][1]);
    while (strcmp(wingdi_get(win, "state"), "closed") != 0) {
        if (demo.change_shader) {
            tinygl_set(gl, "shader.vertex", s_shader_list[curshader][0]);
            tinygl_set(gl, "shader.fragmt", s_shader_list[curshader][1]);
            demo.change_shader = 0, curshader = (curshader + 1) % ARRAYSIZE(s_shader_list);
        }

        matmodel= mat4f_rotate_y(angle * 2 * M_PI / 360); angle += 2;
        matview = mat4f_lookat(demo.camera_position, demo.camera_target, demo.camera_up);
        tinygl_set(gl, "shader.mat_model", &matmodel);
        tinygl_set(gl, "shader.mat_view" , &matview );
        tinygl_begin(gl, 1); srand(0);
        for (i = 0; i < ARRAYSIZE(models); i++) {
            tinygl_set (gl, "shader.texture", model_get_texture(models[i]));
            tinygl_draw(gl, models[i]);
        }
        tinygl_end(gl);
        frame_rate_ctrl(frc, 60);
    }
    for (i = 0; i < ARRAYSIZE(models); i++) { model_free(models[i]); models[i] = NULL; }

    tinygl_free(gl);
    wingdi_free(win, 0);
    return 0;
}
