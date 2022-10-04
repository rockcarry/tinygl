#ifndef _TINYGL_DEFINES_H_
#define _TINYGL_DEFINES_H_

#define ALIGN(a, b) (((a) + (b) - 1) & ~((b) - 1))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

#define offset_of(type, member) ((int)&((type*)0)->member)
#define container_of(ptr, type, member) (type*)((char*)(ptr) - offset_of(type, member))

#endif
