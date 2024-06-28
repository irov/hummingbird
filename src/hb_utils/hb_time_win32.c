#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    __time64_t t = _time64( HB_NULLPTR );

    *_time = (hb_time_t)t;
}

hb_result_t hb_monotonic( hb_time_t * _time )
{
    struct timespec tv;
    if( timespec_get( &tv, TIME_UTC ) != TIME_UTC )
    {
        *_time = 0;

        return HB_FAILURE;
    }

    uint64_t milliseconds = tv.tv_sec * 1000LL + tv.tv_nsec / 1000000LL;

    *_time = (hb_time_t)milliseconds;

    return HB_SUCCESSFUL;
}