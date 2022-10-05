#ifndef _TINYGL_UTILS_H_
#define _TINYGL_UTILS_H_

#include <stdint.h>

uint32_t get_tick_count(void);

int frame_rate_control(uint32_t frc[4], int frate);

#endif
