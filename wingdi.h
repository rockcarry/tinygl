#ifndef __WINGDI_H__
#define __WINGDI_H__

void* wingdi_init(int w, int h);
void  wingdi_free(void *ctx, int close);
void  wingdi_set (void *ctx, char *name, void *data);
void* wingdi_get (void *ctx, char *name);

#endif
