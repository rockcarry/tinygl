#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"

uint32_t get_tick_count(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int frame_rate_ctrl(uint32_t frc[4], int frate)
{
    uint32_t *last_tick = frc + 0;
    uint32_t *next_tick = frc + 1;
    uint32_t *frame_cnt = frc + 2;
    uint32_t *actualrate= frc + 3;
    uint32_t  cur_tick  = get_tick_count();
    int32_t   sleep_tick;
    if (abs((int32_t)*next_tick - (int32_t)cur_tick) > 1000) {
        *last_tick = cur_tick;
        *next_tick = cur_tick + 1000 / frate;
        *frame_cnt = *actualrate = 0;
    } else {
       (*frame_cnt)++;
        if ((int32_t)cur_tick - *last_tick > 1000) {
            *last_tick += 1000;
            *actualrate = *frame_cnt;
            *frame_cnt  = 0;
            printf("actual frame rate: %d\n", *actualrate); fflush(stdout);
        }
        sleep_tick = (int32_t)*next_tick - (int32_t)cur_tick;
        *next_tick+= 1000 / frate;
        if (sleep_tick > 0) usleep(sleep_tick * 1000);
    }
    return *actualrate;
}
