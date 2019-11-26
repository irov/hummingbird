#include "hb_time.h"

#define _LARGE_TIME_API
#include <time.h>

void hb_time( hb_time_t * _time )
{
    time64_t time = time64( HB_NULLPTR );

    *_time = (hb_time_t)time;
}