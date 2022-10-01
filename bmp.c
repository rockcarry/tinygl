#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"

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
int bmp_create(BMP *pb, int w, int h, int cdepth)
{
    if (cdepth != 24 && cdepth != 32) cdepth = 24;
    pb->width  = w;
    pb->height = h;
    pb->cdepth = cdepth;
    pb->stride = ALIGN(pb->width * (pb->cdepth / 8), 4);
    pb->pdata  = calloc(1, pb->height * pb->stride);
    return pb->pdata ? 0 : -1;
}

void bmp_destroy(BMP *pb)
{
    free(pb->pdata);
    memset(pb, 0, sizeof(BMP));
}

int bmp_load(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint8_t      *pdata  = NULL;
    int           i;

    fp = fopen(file, "rb");
    if (!fp) return -1;

    fread(&header, sizeof(header), 1, fp);
    pb->width  = header.biWidth;
    pb->height = header.biHeight;
    pb->stride = ALIGN(header.biWidth * 3, 4);
    pb->cdepth = 24;
    pb->pdata  = malloc(pb->stride * pb->height);
    if (pb->pdata) {
        pdata  = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            pdata -= pb->stride;
            fread(pdata, pb->stride, 1, fp);
        }
    }

    fclose(fp);
    return pb->pdata ? 0 : -1;
}

int bmp_save(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint8_t      *pdata;
    int           i;

    header.bfType     = ('B' << 0) | ('M' << 8);
    header.bfSize     = sizeof(header) + pb->stride * pb->height;
    header.bfOffBits  = sizeof(header);
    header.biSize     = 40;
    header.biWidth    = pb->width;
    header.biHeight   = pb->height;
    header.biPlanes   = 1;
    header.biBitCount = pb->cdepth;
    header.biSizeImage= pb->stride * pb->height;

    fp = fopen(file, "wb");
    if (fp) {
        fwrite(&header, sizeof(header), 1, fp);
        pdata = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            pdata -= pb->stride;
            fwrite(pdata, pb->stride, 1, fp);
        }
        fclose(fp);
    }

    return fp ? 0 : -1;
}

void bmp_setrgb(BMP *pb, int x, int y, uint8_t  r, uint8_t  g, uint8_t  b)
{
    if (x < 0 || y < 0 || x >= pb->width || y >= pb->height) return;
    pb->pdata[x * (pb->cdepth / 8) + 0 + y * pb->stride] = b;
    pb->pdata[x * (pb->cdepth / 8) + 1 + y * pb->stride] = g;
    pb->pdata[x * (pb->cdepth / 8) + 2 + y * pb->stride] = r;
}

void bmp_getrgb(BMP *pb, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (x < 0 || y < 0 || x >= pb->width || y >= pb->height) { *r = *g = *b = 0; return; }
    *b = pb->pdata[x * (pb->cdepth / 8) + 0 + y * pb->stride];
    *g = pb->pdata[x * (pb->cdepth / 8) + 1 + y * pb->stride];
    *r = pb->pdata[x * (pb->cdepth / 8) + 2 + y * pb->stride];
}

void bmp_setpixel(BMP *pb, int x, int y, int c)
{
    if (x < 0 || y < 0 || x >= pb->width || y >= pb->height) return;
    pb->pdata[x * (pb->cdepth / 8) + 0 + y * pb->stride] = c >> 0 ;
    pb->pdata[x * (pb->cdepth / 8) + 1 + y * pb->stride] = c >> 8 ;
    pb->pdata[x * (pb->cdepth / 8) + 2 + y * pb->stride] = c >> 16;
}

int bmp_getpixel(BMP *pb, int x, int y)
{
    uint8_t r, g, b;
    if (x < 0 || y < 0 || x >= pb->width || y >= pb->height) return 0;
    b = pb->pdata[x * (pb->cdepth / 8) + 0 + y * pb->stride];
    g = pb->pdata[x * (pb->cdepth / 8) + 1 + y * pb->stride];
    r = pb->pdata[x * (pb->cdepth / 8) + 2 + y * pb->stride];
    return RGB(r, g, b);
}

void bmp_line(BMP *bmp, int x1, int y1, int x2, int y2, int c)
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
            bmp_setpixel(bmp, x1, y1, c);
        }
    } else {
        d = x2 - x1 > 0 ? 1 : -1;
        for (e = dy/2; y1 < y2; y1++, e += dx) {
            if (e >= dy) e -= dy, x1 += d;
            bmp_setpixel(bmp, x1, y1, c);
        }
    }
    bmp_setpixel(bmp, x2, y2, c);
}

void bmp_bitblt(BMP *pbdst, int dstx, int dsty, BMP *pbsrc, int srcx, int srcy, int w, int h)
{
    int  i, j;
    w = w > 0 ? w : pbsrc->width;
    h = h > 0 ? h : pbsrc->height;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            bmp_setpixel(pbdst, dstx + j, dsty + i, bmp_getpixel(pbsrc, srcx + j, srcy + i));
        }
    }
}

void bmp_fillrect(BMP *pb, int x, int y, int w, int h, int c)
{
    int a = c >> 24, i, j;
    if (a) {
        uint8_t r = c >> 16, g = c >> 8, b = c >> 0, fr, fg, fb, br, bg, bb;
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                c  = bmp_getpixel(pb, x + j, y + i);
                br = c >> 16;
                bg = c >> 8 ;
                bb = c >> 0 ;
                fr = r + a * (br - r) / 255;
                fg = g + a * (bg - g) / 255;
                fb = b + a * (bb - b) / 255;
                bmp_setpixel(pb, x + j, y + i, RGB(fr, fg, fb));
            }
        }
    } else {
        for (i = 0; i < h; i++) {
            for (j = 0; j < w; j++) {
                bmp_setpixel(pb, x + j, y + i, c);
            }
        }
    }
}

#if _TEST_BMP_
int main(int argc, char *argv[])
{
    BMP mybmp1 = {};
    BMP mybmp2 = {};
    bmp_load(&mybmp1, "test.bmp");
    bmp_load(&mybmp2, "bb.bmp");
    bmp_bitblt(&mybmp1, 0, 0, &mybmp2, 0, 0, -1, -1);
    bmp_fillrect(&mybmp1, 50, 50, 200, 200, RGB(0, 0, 255) | (80 << 24));
    bmp_save(&mybmp1, "out.bmp");
    bmp_destroy(&mybmp1);
    bmp_destroy(&mybmp2);
    return 0;
}
#endif
