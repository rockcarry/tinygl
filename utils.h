#ifndef _TINYGL_UTILS_H_
#define _TINYGL_UTILS_H_

#include <stdint.h>

#define ALIGN(a, b)  (((a) + (b) - 1) & ~((b) - 1))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) < (b) ? (a) : (b))
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

#define offset_of(type, member) ((size_t)&((type*)0)->member)
#define container_of(ptr, type, member) (type*)((char*)(ptr) - offset_of(type, member))

uint32_t get_tick_count(void);

int frame_rate_ctrl(uint32_t frc[4], int frate);

#endif
