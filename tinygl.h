#ifndef _TINYGL_H_
#define _TINYGL_H_

void* tinygl_init (int w, int h, void *shader);
void  tinygl_free (void *ctx);
void  tinygl_draw (void *ctx, void *model);
void  tinygl_clear(void *ctx, char *type );
void  tinygl_save_screen (void *ctx, char *file);
void* tinygl_get_screen  (void *ctx);
void  tinygl_set_matrix  (void *ctx, char *type, float *matrix);
void* tinygl_get_matrix  (void *ctx, char *type);
void  tinygl_set_shader  (void *ctx, void *shader);
void  tinygl_set_viewport(void *ctx, int x, int y, int w, int h, int depth);

#endif
