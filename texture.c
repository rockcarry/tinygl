#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "texture.h"

#define ALIGN(a, b) (((a) + (b) - 1) & ~((b) - 1))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

//++ for bmp file ++//
// 内部类型定义
#pragma pack(1)
typedef struct {
    uint16_t  bfType;
    uint32_t  bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;
    uint32_t  biSize;
    uint32_t  biWidth;
    uint32_t  biHeight;
    uint16_t  biPlanes;
    uint16_t  biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    uint32_t  biXPelsPerMeter;
    uint32_t  biYPelsPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImportant;
} BMPFILEHEADER;
#pragma pack()

/* 函数实现 */
TEXTURE* texture_create(int w, int h, int cdepth)
{
    TEXTURE *text;
    if (cdepth != 24 && cdepth != 32) cdepth = 24;
    text = malloc(sizeof(TEXTURE) + ALIGN(w * (cdepth / 8), 4) * h);
    if (!text) return NULL;
    text->width  = w;
    text->height = h;
    text->cdepth = cdepth;
    text->stride = ALIGN(text->width * (text->cdepth / 8), 4);
    text->pdata  = (uint8_t*)(text + 1);
    return text;
}

void texture_destroy(TEXTURE *t) { free(t); }

TEXTURE* texture_load(char *file)
{
    TEXTURE      *texture= NULL;
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    int           i;

    fp = fopen(file, "rb");
    if (!fp) return NULL;

    fread(&header, sizeof(header), 1, fp);
    texture = texture_create(header.biWidth, header.biHeight, header.biBitCount);
    if (texture) {
        uint8_t *pdata = texture->pdata + texture->stride * texture->height;
        for (i = 0; i < texture->height; i++) {
            pdata -= texture->stride;
            fread(pdata, texture->stride, 1, fp);
        }
    }

    fclose(fp);
    return texture;
}

int texture_save(TEXTURE *t, char *file)
{
    FILE         *fp     = fopen(file, "wb");
    BMPFILEHEADER header = {0};
    uint8_t      *pdata;
    int           i;

    if (!fp) return -1;
    header.bfType     = ('B' << 0) | ('M' << 8);
    header.bfSize     = sizeof(header) + t->stride * t->height;
    header.bfOffBits  = sizeof(header);
    header.biSize     = 40;
    header.biWidth    = t->width;
    header.biHeight   = t->height;
    header.biPlanes   = 1;
    header.biBitCount = t->cdepth;
    header.biSizeImage= t->stride * t->height;
    fwrite(&header, sizeof(header), 1, fp);
    pdata = (uint8_t*)t->pdata + t->stride * t->height;
    for (i = 0; i < t->height; i++) {
        pdata -= t->stride;
        fwrite(pdata, t->stride, 1, fp);
    }

    fclose(fp);
    return 0;
}

void texture_setrgb(TEXTURE *t, int x, int y, uint8_t  r, uint8_t  g, uint8_t  b)
{
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return;
    t->pdata[x * (t->cdepth / 8) + 0 + y * t->stride] = b;
    t->pdata[x * (t->cdepth / 8) + 1 + y * t->stride] = g;
    t->pdata[x * (t->cdepth / 8) + 2 + y * t->stride] = r;
}

void texture_getrgb(TEXTURE *t, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) { *r = *g = *b = 0; return; }
    *b = t->pdata[x * (t->cdepth / 8) + 0 + y * t->stride];
    *g = t->pdata[x * (t->cdepth / 8) + 1 + y * t->stride];
    *r = t->pdata[x * (t->cdepth / 8) + 2 + y * t->stride];
}

void texture_setcolor(TEXTURE *t, int x, int y, uint32_t c)
{
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return;
    t->pdata[x * (t->cdepth / 8) + 0 + y * t->stride] = c >> 0 ;
    t->pdata[x * (t->cdepth / 8) + 1 + y * t->stride] = c >> 8 ;
    t->pdata[x * (t->cdepth / 8) + 2 + y * t->stride] = c >> 16;
}

uint32_t texture_getcolor(TEXTURE *t, int x, int y)
{
    uint8_t r, g, b;
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return 0;
    b = t->pdata[x * (t->cdepth / 8) + 0 + y * t->stride];
    g = t->pdata[x * (t->cdepth / 8) + 1 + y * t->stride];
    r = t->pdata[x * (t->cdepth / 8) + 2 + y * t->stride];
    return RGB(r, g, b);
}

void texture_line(TEXTURE *t, int x1, int y1, int x2, int y2, uint32_t c)
{
    int dx, dy, d, e;

    dx = abs(x1 - x2);
    dy = abs(y1 - y2);

    if ((dx >= dy && x1 > x2) || (dx < dy && y1 > y2)) {
        d = x1; x1 = x2; x2 = d;
        d = y1; y1 = y2; y2 = d;
    }

    if (dx >= dy) {
        d = y2 - y1 > 0 ? 1 : -1;
        for (e = dx/2; x1 < x2; x1++, e += dy) {
            if (e >= dx) e -= dx, y1 += d;
            texture_setcolor(t, x1, y1, c);
        }
    } else {
        d = x2 - x1 > 0 ? 1 : -1;
        for (e = dy/2; y1 < y2; y1++, e += dx) {
            if (e >= dy) e -= dy, x1 += d;
            texture_setcolor(t, x1, y1, c);
        }
    }
    texture_setcolor(t, x2, y2, c);
}

void texture_bitblt(TEXTURE *dst, int dstx, int dsty, TEXTURE *src, int srcx, int srcy, int w, int h)
{
    int  i, j;
    w = w > 0 ? w : src->width;
    h = h > 0 ? h : src->height;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            texture_setcolor(dst, dstx + j, dsty + i, texture_getcolor(src, srcx + j, srcy + i));
        }
    }
}

void texture_fillrect(TEXTURE *t, int x, int y, int w, int h, uint32_t c)
{
    int a = c >> 24, i, j;
    if (a) {
        uint8_t r = c >> 16, g = c >> 8, b = c >> 0, fr, fg, fb, br, bg, bb;
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                texture_getrgb(t, x + j, y + i, &br, &bg, &bb);
                fr = r + a * (br - r) / 255;
                fg = g + a * (bg - g) / 255;
                fb = b + a * (bb - b) / 255;
                texture_setrgb(t, x + j, y + i, fr, fg, fb);
            }
        }
    } else {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                texture_setcolor(t, x + j, y + i, c);
            }
        }
    }
}

#if _TEST_TEXTURE_
int main(int argc, char *argv[])
{
    TEXTURE *texture1 = texture_load("test.bmp");
    TEXTURE *texture2 = texture_load("bb.bmp"  );

    texture_bitblt(texture1, 0, 0, texture2, 0, 0, -1, -1);
    texture_fillrect(texture1, 50, 50, 200, 200, RGB(0, 0, 255) | (80 << 24));
    texture_save(texture1, "out.bmp");
    texture_destroy(texture1);
    texture_destroy(texture2);
    return 0;
}
#endif
