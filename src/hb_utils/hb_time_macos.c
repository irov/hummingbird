#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    time_t t = time( HB_NULLPTR );

    *_time = (hb_time_t)t;
}

hb_result_t hb_monotonic( hb_time_t * _time )
{
    struct timespec ts;
    if( clock_gettime( CLOCK_MONOTONIC, &ts ) == -1 )
    {
        *_time = 0;

        return HB_FAILURE;
    }

    uint64_t milliseconds = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;

    *_time = (hb_time_t)milliseconds;

    return HB_SUCCESSFUL;
}