#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    time_t t = time( HB_NULLPTR );

    *_time = (hb_time_t)t;
}