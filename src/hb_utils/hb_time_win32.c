#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    __time64_t t = _time64( HB_NULLPTR );

    *_time = (hb_time_t)t;
}