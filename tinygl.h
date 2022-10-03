#ifndef _TINYGL_H_
#define _TINYGL_H_

void* tinygl_init (int w, int h, void *shader);
void  tinygl_free (void *ctx);
void  tinygl_draw (void *ctx, void *model);
void  tinygl_clear(void *ctx, char *type );
void  tinygl_set_viewport(void *ctx, int x, int y, int w, int h, int depth);
void  tinygl_set_param(void *ctx, char *name, void *data);
void* tinygl_get_param(void *ctx, char *name);

#endif
