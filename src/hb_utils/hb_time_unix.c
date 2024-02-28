#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    time_t t = time( HB_NULLPTR );

    *_time = (hb_time_t)t;
}

void hb_monotonic( hb_time_t * _time )
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );

    uint64_t milliseconds = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;

    *_time = (hb_time_t)milliseconds;
}