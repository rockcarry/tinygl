#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "texture.h"
#include "utils.h"

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

#pragma pack(1)
typedef struct {
    uint8_t   IDLength;
    uint8_t   ColorMapType;
    uint8_t   ImageType;
    uint16_t  CMapStart;
    uint16_t  CMapLength;
    uint8_t   CMapDepth;
    uint16_t  XOffset;
    uint16_t  YOffset;
    uint16_t  Width;
    uint16_t  Height;
    uint8_t   PixelDepth;
    uint8_t   ImageDesc;
} TGAFILEHEADER;
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

static TEXTURE* texture_load_bmp(char *file)
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

static TEXTURE* texture_load_tga(char *file)
{
    TEXTURE      *texture= NULL;
    TGAFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint32_t      pal[256], val32 = 0;
    uint16_t      val16;
    uint8_t       hpkt, r, g, b;
    int           n, i, j, x, y;

    fp = fopen(file, "rb");
    if (!fp) return NULL;

    fread(&header, sizeof(header), 1, fp);
    texture = texture_init(header.Width, header.Height);
    if (texture) {
        if (header.ColorMapType) {
            fseek(fp, header.CMapStart, SEEK_SET);
            n = MIN(header.CMapLength, 256);
            for (i = 0; i < n; i++) {
                switch (header.CMapDepth) {
                case 16:
                    fread(&val16, sizeof(val16), 1, fp);
                    r = (val32 >> 8) & 0xF8;
                    g = (val32 >> 3) & 0xFC;
                    b = (val32 << 3) & 0xF8;
                    pal[i] = RGB(r, g, b);
                    break;
                case 24:
                    r = fgetc(fp);
                    g = fgetc(fp);
                    b = fgetc(fp);
                    pal[i] = RGB(r, g, b);
                    break;
                case 32:
                    fread(pal + i, sizeof(uint32_t), 1, fp);
                    break;
                }
            }
        }
        fseek(fp, sizeof(header) + header.IDLength + (!!header.ColorMapType) * header.CMapLength * header.CMapDepth / 8, SEEK_SET);
        if (header.PixelDepth > 32) header.PixelDepth = 32;
        switch (header.ImageType) {
        case 1: case 9: case 2: case 10:
            for (n = texture->w * texture->h, i = 0; i < n && !feof(fp); ) {
                hpkt = (header.ImageType == 9 || header.ImageType == 10) ? fgetc(fp) : 0;
                if (hpkt & (1 << 7)) fread(&val32, header.PixelDepth / 8, 1, fp);
                for (j = 0; j < (hpkt & 0x7F) + 1; j++, i++) {
                    if (!(hpkt & (1 << 7))) fread(&val32, header.PixelDepth / 8, 1, fp);
                    if (header.ImageType == 1 || header.ImageType == 9) {
                        val32 = pal[val32 % 256];
                    } else if (header.PixelDepth == 16) {
                        r = (val32 >> 8) & 0xF8;
                        g = (val32 >> 3) & 0xFC;
                        b = (val32 << 3) & 0xF8;
                        val32 = RGB(r, g, b);
                    }
                    x = i % texture->w;
                    x = header.ImageDesc & (1 << 4) ? texture->w - 1 - x : x;
                    y = i / texture->w;
                    y = header.ImageDesc & (1 << 5) ? texture->h - 1 - y : y;
                    texture_setcolor(texture, x, y, val32);
                }
            }
            break;
        }
    }

    fclose(fp);
    return texture;
}

static int texture_save_bmp(TEXTURE *t, char *file)
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

static int texture_save_tga(TEXTURE *t, char *file)
{
    FILE         *fp     = fopen(file, "wb");
    TGAFILEHEADER header = {0};
    uint32_t     *data;

    if (!fp) return -1;
    header.ImageType  = 2;
    header.Width      = t->w;
    header.Height     = t->h;
    header.PixelDepth = 24;
    fwrite(&header, sizeof(header), 1, fp);
    data = t->data + t->w * (t->h - 1);
    for (int i = 0; i < t->h; i++) {
        for (int j = 0; j < t->w; j++) {
            fputc(data[j] >> 0, fp);
            fputc(data[j] >> 8, fp);
            fputc(data[j] >>16, fp);
        }
        data -= t->w;
    }
    fclose(fp);
    return 0;
}

TEXTURE* texture_load(char *file)
{
    if (file) {
        int   len     = strlen(file);
        char *postfix = file + MAX(len - 4, 0);
        if (strcasecmp(postfix, ".bmp") == 0) return texture_load_bmp(file);
        if (strcasecmp(postfix, ".tga") == 0) return texture_load_tga(file);
    }
    return NULL;
}

int texture_save(TEXTURE *t, char *file)
{
    if (t && file) {
        int   len     = strlen(file);
        char *postfix = file + MAX(len - 4, 0);
        if (strcasecmp(postfix, ".bmp") == 0) return texture_save_bmp(t, file);
        if (strcasecmp(postfix, ".tga") == 0) return texture_save_tga(t, file);
    }
    return -1;
}

void texture_lock  (TEXTURE *t) { if (t && t->lock  ) t->lock  (t); }
void texture_unlock(TEXTURE *t) { if (t && t->unlock) t->unlock(t); }

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
    if (!t) return;

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
    if (!dst || !src) return;
    w = w > 0 ? w : src->w;
    h = h > 0 ? h : src->h;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            texture_setcolor(dst, dstx + j, dsty + i, texture_getcolor(src, srcx + j, srcy + i));
        }
    }
}

static uint32_t alpha_blend(uint32_t a, uint32_t b, uint8_t alpha)
{
    uint32_t a_br  = a & 0xFF00FF;
    uint32_t b_br  = b & 0xFF00FF;
    uint32_t a_g   = a & 0x00FF00;
    uint32_t b_g   = b & 0x00FF00;
    uint32_t c_br  = b_br + alpha * (a_br - b_br) / 256;
    uint32_t c_g   = b_g  + alpha * (a_g  - b_g ) / 256;
    return (c_br & (0xFF00FF)) | (c_g & 0x00FF00);
}

void texture_fillrect(TEXTURE *t, int x, int y, int w, int h, uint32_t c)
{
    uint8_t alpha = c >> 24;
    if (!t) return;
    if (alpha) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                texture_setcolor(t, x + j, y + i, alpha_blend(texture_getcolor(t, x + j, y + i), c, alpha));
            }
        }
    } else {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
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
