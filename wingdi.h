#ifndef __WINGDI_H__
#define __WINGDI_H__

#include <stdint.h>

enum {
    WINGDI_MSG_KEY_EVENT = 1,
    WINGDI_MSG_MOUSE_EVENT,
};

typedef void (*PFN_WIN_MSG_CB)(void *cbctx, int msg, uint32_t param1, uint32_t param2, void *param3);

void* wingdi_init(int w, int h, PFN_WIN_MSG_CB callback, void *cbctx);
void  wingdi_free(void *ctx, int close);
void  wingdi_set (void *ctx, char *name, void *data);
void* wingdi_get (void *ctx, char *name);

#endif
