#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdint.h>

#ifndef RGB
#define RGB(r, g, b) (((unsigned)(r) << 16) | ((unsigned)(g) << 8) | ((unsigned)(b) << 0))
#endif

typedef struct texture_t {
    int       w, h; /* 宽高 */
    uint32_t *data; /* 指向数据 */
    void (*lock  )(struct texture_t *t);
    void (*unlock)(struct texture_t *t);
} TEXTURE;

TEXTURE* texture_init(int w, int h);
void     texture_free(TEXTURE *t);

TEXTURE* texture_load(char *file);
int      texture_save(TEXTURE *t, char *file);

void     texture_lock  (TEXTURE *t);
void     texture_unlock(TEXTURE *t);

void     texture_setcolor(TEXTURE *t, int x, int y, uint32_t c);
uint32_t texture_getcolor(TEXTURE *t, int x, int y);

void texture_line    (TEXTURE *t, int x1, int y1, int x2, int y2, uint32_t c);
void texture_bitblt  (TEXTURE *dst, int dstx, int dsty, TEXTURE *src, int srcx, int srcy, int w, int h);
void texture_fillrect(TEXTURE *t, int x, int y, int w, int h, uint32_t c);

#endif
