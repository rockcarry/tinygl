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
TEXTURE* texture_init(int w, int h)
{
    TEXTURE *text;
    if (w <= 0) w = 8;
    if (h <= 0) h = 8;
    text = malloc(sizeof(TEXTURE) + w * h * sizeof(uint32_t));
    if (!text) return NULL;
    text->w    = w;
    text->h    = h;
    text->data = (uint32_t*)(text + 1);
    return text;
}

void texture_free(TEXTURE *t) { free(t); }

TEXTURE* texture_load(char *file)
{
    TEXTURE      *texture= NULL;
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    int           i, j;

    fp = fopen(file, "rb");
    if (!fp) return NULL;

    fread(&header, sizeof(header), 1, fp);
    texture = texture_init(header.biWidth, header.biHeight);
    if (texture) {
        uint32_t *data = texture->data + texture->w * (texture->h - 1);
        int       skip = (4 - ((header.biWidth * header.biBitCount + 7) / 8) % 4) % 4;
        uint8_t   pal[256 * 4], c1 = 0, c2;
        if (header.biBitCount == 4 || header.biBitCount == 8) {
            fseek(fp, sizeof(BMPFILEHEADER), SEEK_SET);
            fread(pal, 1, (1 << header.biBitCount) * 4, fp);
        }
        fseek(fp, header.bfOffBits, SEEK_SET);
        for (i = 0; i < texture->h; i++) {
            for (j = 0; j < texture->w; j++) {
                uint8_t argb[4] = {};
                switch (header.biBitCount) {
                case 1:
                    if (!(j & 7)) c1 = fgetc(fp);
                    argb[0] = argb[1] = argb[2] = (c1 & (1 << 7)) ? 255 : 0;
                    c1 <<= 1;
                    break;
                case 4: case 8:
                    if (header.biBitCount == 4) {
                        if (!(j & 1)) c1 = fgetc(fp);
                        c2 = c1 >> 4;
                        c1 = c1 << 4;
                    } else c2 = fgetc(fp);
                    argb[0] = pal[c2 * 4 + 0]; argb[1] = pal[c2 * 4 + 1]; argb[2] = pal[c2 * 4 + 2];
                    break;
                case 24: argb[0] = fgetc(fp); argb[1] = fgetc(fp); argb[2] = fgetc(fp); break;
                case 32: argb[0] = fgetc(fp); argb[1] = fgetc(fp); argb[2] = fgetc(fp); argb[3] = fgetc(fp); break;
                }
                *data++ = *(uint32_t*)argb;
            }
            data -= 2 * texture->w;
            fseek(fp, skip, SEEK_CUR);
        }
    }

    fclose(fp);
    return texture;
}

int texture_save(TEXTURE *t, char *file)
{
    FILE         *fp     = fopen(file, "wb");
    BMPFILEHEADER header = {0};
    uint8_t      *data   = NULL;
    int           skip, i, j;

    if (!fp) return -1;
    header.bfType     = ('B' << 0) | ('M' << 8);
    header.bfSize     = sizeof(header) + ALIGN(t->w * 3, 4) * t->h;
    header.bfOffBits  = sizeof(header);
    header.biSize     = 40;
    header.biWidth    = t->w;
    header.biHeight   = t->h;
    header.biPlanes   = 1;
    header.biBitCount = 24;
    header.biSizeImage= ALIGN(t->w * 3, 4) * t->h;
    fwrite(&header, sizeof(header), 1, fp);
    data = (uint8_t*)(t->data + t->w * (t->h - 1));
    skip = (4 - (t->w * header.biBitCount / 8) % 4) % 4;
    for (i = 0; i < t->h; i++) {
        for (j = 0; j < t->w; j++) {
            fputc(*data++, fp);
            fputc(*data++, fp);
            fputc(*data++, fp);
            data++;
        }
        fseek(fp, skip, SEEK_CUR);
        data -= 2 * t->w * sizeof(uint32_t);
    }
    fclose(fp);
    return 0;
}

void texture_setcolor(TEXTURE *t, int x, int y, uint32_t c)
{
    if ((unsigned)x < t->w && (unsigned)y < t->h) t->data[x + y * t->w] = c;
}

uint32_t texture_getcolor(TEXTURE *t, int x, int y)
{
    return ((unsigned)x < t->w && (unsigned)y < t->h) ? t->data[x + y * t->w] : 0;
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
    w = w > 0 ? w : src->w;
    h = h > 0 ? h : src->h;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            texture_setcolor(dst, dstx + j, dsty + i, texture_getcolor(src, srcx + j, srcy + i));
        }
    }
}

void texture_fillrect(TEXTURE *t, int x, int y, int w, int h, uint32_t c)
{
    uint8_t *c_argb = (uint8_t*)&c;
    int      i, j;
    if (c_argb[3]) {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                uint32_t bc = texture_getcolor(t, x + j, y + i);
                uint8_t *b_argb = (uint8_t*)&bc;
                b_argb[2] = c_argb[2] + c_argb[3] * (b_argb[2] - c_argb[2]) / 255;
                b_argb[1] = c_argb[1] + c_argb[3] * (b_argb[1] - c_argb[1]) / 255;
                b_argb[0] = c_argb[0] + c_argb[3] * (b_argb[0] - c_argb[0]) / 255;
                texture_setcolor(t, x + j, y + i, bc);
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
    TEXTURE *texture2 = texture_load("hmbb.bmp");

    texture_bitblt(texture1, 0, 0, texture2, 0, 0, -1, -1);
    texture_fillrect(texture1, 50, 50, 200, 200, RGB(0, 0, 255) | (80 << 24));
    texture_save(texture1, "out.bmp");
    texture_free(texture1);
    texture_free(texture2);
    return 0;
}
#endif
