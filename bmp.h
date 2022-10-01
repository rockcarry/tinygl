#ifndef __BMP_H__
#define __BMP_H__

#define RGB(r, g, b) (((unsigned)(r) << 16) | ((unsigned)(g) << 8) | ((unsigned)(b) << 0))

/* BMP 对象的类型定义 */
typedef struct {
    int   width;   /* 宽度 */
    int   height;  /* 高度 */
    int   stride;  /* 行字节数 */
    int   cdepth;  /* 像素位数 */
    char *pdata;   /* 指向数据 */
} BMP;

int  bmp_create (BMP *pb, int w, int h, int cdepth);
void bmp_destroy(BMP *pb);

int  bmp_load(BMP *pb, char *file);
int  bmp_save(BMP *pb, char *file);

void bmp_setpixel(BMP *pb, int x, int y, int c);
int  bmp_getpixel(BMP *pb, int x, int y);
void bmp_line    (BMP *pb, int x1, int y1, int x2, int y2, int c);
void bmp_bitblt  (BMP *pbdst, int dstx, int dsty, BMP *pbsrc, int srcx, int srcy, int w, int h);
void bmp_fillrect(BMP *pb, int x, int y, int w, int h, int c);

#endif
