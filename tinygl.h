#ifndef _TINYGL_H_
#define _TINYGL_H_

void* tinygl_init (int w, int h);
void  tinygl_free (void *ctx);
void  tinygl_begin(void *ctx, int clear);
void  tinygl_end  (void *ctx);
void  tinygl_draw (void *ctx, void *model);
void  tinygl_clear(void *ctx, char *type );
void  tinygl_viewport(void *ctx, int x, int y, int w, int h, int depth);
void  tinygl_set(void *ctx, char *name, void *data);
void* tinygl_get(void *ctx, char *name);

#endif
