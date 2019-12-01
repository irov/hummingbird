#include "hb_date.h"
#include "hb_clock.h"

#include <time.h>

void hb_date( hb_date_t * _date )
{
    time_t t = time( HB_NULLPTR );
    struct tm * tm = localtime( &t );

    _date->year = tm->tm_year + 1900;
    _date->mon = tm->tm_mon + 1;
    _date->mday = tm->tm_mday;
    _date->hour = tm->tm_hour;
    _date->min = tm->tm_min;
    _date->sec = tm->tm_sec;

    hb_clock_t msec;
    hb_clock_msec( &msec );

    _date->msec = msec;
}