+----------------+
 tinygl 3d 渲染器
+----------------+

tinygl 是一个软件光栅化 3D 图形渲染器。

在学习了 TinyRenderer 后，自己使用 C 语言写了这个 tinygl.
代码使用纯 C 编写，渲染部分的代码不依赖于任何第三方库和平台 API，只需要有标准 C 库的支持即可编译通过

渲染结果可保存到 .bmp 和 .tga 图片，也可以直接显示在 windows 的窗口上。
目前 windows 平台支持 wingdi 用于实现渲染结果输出到窗口，方便查看结果。

1000 行代码，基本就实现了 TinyRenderer 的完整功能，支持的特性如下：
1. TGA 图片和 BMP 图片的加载和保存
2. 基础的像素点绘制、直线绘制、bitblt 图块绘制、填充矩形绘制
3. vector 向量和 matrix 矩阵的基础运算功能
4. obj 格式的模型文件的加载和保存以及数据的读取
5. 基于包围盒和重心坐标的三角形光栅化算法的实现
6. 渲染管线的实现：背面剔除、z-buffer、纹理映射等
7. 顶点着色器：wire, rand, flat, gouraud
8. 片元着色器：color*, phong, normal*, texture*
9. 简洁易用的 API 设计、优化的架构设计、良好的编码规范



+--------+
 API 说明
+--------+

model.h
-------
用于模型加载，并从中获取模型数据，包括：三角形数据和纹理数据
void* model_load(char *object, char *texture);
指定模型的 .obj 和 .tga 文件，加载模型并得到对象指针

void model_save(void *ctx, char *object, char *texture);
保存模型到 .obj 和 .tga 文件

void model_free(void *ctx);
释放模型对象

void* model_get_texture(void *ctx);
得到模型的 texture 纹理数据

int model_get_face(void *ctx, int idx, vertex_t face[3]);
根据 idx 编号，获取模型的三角形数据


tinygl.h
--------
用于渲染模型，创建 后内部会创建一个 texture 对象作为 render target
也可以通过 tinygl_set(gl, "target", texture); 来指定一个新的 texture 作为 target
通过 tinygl_set(gl, "save", “out.bmp”); 可以把渲染结果保存到文件进行查看

void* tinygl_init(int w, int h);
初始化 tinygl 并得到 tinygl 的对象指针

void tinygl_free(void *ctx);
反初始化

void tinygl_begin(void *ctx);
开始渲染

void tinygl_end(void *ctx);
完成渲染

void tinygl_draw(void *ctx, void *model);
放到 tinygl_begin 和 tinygl_end 中间，用于绘制模型

void tinygl_clear(void *ctx, char *type);
用于清除 frame buffer 或者 z-buffer
tinygl_clear(gl, "framebuf"); // 清除 frame buffer
tinygl_clear(gl, "zbuffer" ); // 清除 z-buffer

void  tinygl_set(void *ctx, char *name, void *data);
void* tinygl_get(void *ctx, char *name);
用于获取和设置渲染参数


wingdi.h
--------
wingdi 实现了 windows 下的 gdi 窗口，创建时内部自动创建了一个 texture 对象，
在这个 texture 对象上绘制图形就等于是在窗口上绘制。

通过 wingdi 的 get 函数调用：
TEXTURE *t = wingdi_get(win, "texture");
获取到这个 texture 对对象，然后使用 texture 的 API 就可以在窗口上绘图了

void* wingdi_init(int w, int h);
初始化并得到 wingdi 对象，w h 指定了窗口大小

void wingdi_free(void *ctx, int close);
反初始化

void  wingdi_set(void *ctx, char *name, void *data);
void* wingdi_get(void *ctx, char *name);
用于获取和设置参数


shader.h
--------
shader 模块定义并实现了 shader 的接口和数据类型，可以在此基础上扩展实现更多的顶点着色器和片元着色器

目前已经实现的着色器有：
顶点着色器：perspective, wire, rand, flat, gouraud
片元着色器：color0, color1, phong, normal0, normal1, texture0, texture1, texture2

用户也可以通过 tinygl_set(gl, "shader.xxx", data); 来设置 tinygl 内部的 shader 参数
通过设置不同的 vertext 顶点着色器和 fragmt 片元着色器，可以得到不同的渲染效果



+------------+
 hello tinygl
+------------+

#include "tinygl.h"
#include "model.h"

int main(void)
{
    void *gl = tinygl_init(800, 800);
    void *m  = model_load("head.obj", NULL);

    tinygl_begin(gl);
    tinygl_draw(gl, m);
    tinygl_end(gl);
    tinygl_set(gl, "save", "out.bmp");

    model_free(m);
    tinygl_free(gl);
    return 0;
}



rockcarry
2022/10/4


