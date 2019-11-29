#include "hb_date.h"

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

    clock_t c = clock();

    clock_t msec = c / (CLOCKS_PER_SEC / 1000);

    _date->msec = msec % 1000;
}