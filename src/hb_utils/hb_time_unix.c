#include "hb_time.h"

#include <time.h>

void hb_time( hb_time_t * _time )
{
    time_t time = time( HB_NULLPTR );

    *_time = (hb_time_t)time;
}