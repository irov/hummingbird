#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    __time64_t time = _time64( HB_NULLPTR );

    *_time = (hb_time_t)time;
}