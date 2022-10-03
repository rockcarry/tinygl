#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdint.h>

#define RGB(r, g, b) (((unsigned)(r) << 16) | ((unsigned)(g) << 8) | ((unsigned)(b) << 0))

/* BMP ��������Ͷ��� */
typedef struct {
    int      width;   /* ���� */
    int      height;  /* �߶� */
    int      stride;  /* ���ֽ��� */
    int      cdepth;  /* ����λ�� */
    uint8_t *pdata;   /* ָ������ */
} TEXTURE;

TEXTURE* texture_create(int w, int h, int cdepth);
void     texture_destroy(TEXTURE *t);

TEXTURE* texture_load(char *file);
int      texture_save(TEXTURE *t, char *file);

void texture_setrgb(TEXTURE *t, int x, int y, uint8_t  r, uint8_t  g, uint8_t  b);
void texture_getrgb(TEXTURE *t, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b);

void     texture_setcolor(TEXTURE *t, int x, int y, uint32_t c);
uint32_t texture_getcolor(TEXTURE *t, int x, int y);

void texture_line    (TEXTURE *t, int x1, int y1, int x2, int y2, uint32_t c);
void texture_bitblt  (TEXTURE *dst, int dstx, int dsty, TEXTURE *src, int srcx, int srcy, int w, int h);
void texture_fillrect(TEXTURE *t, int x, int y, int w, int h, uint32_t c);

#endif