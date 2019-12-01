#include "hb_clock.h"

#include <time.h>

void hb_clock_msec( hb_clock_t * _clock )
{
    clock_t c = clock();

    clock_t msec = c / (CLOCKS_PER_SEC / 1000);

    *_clock = msec % 1000;
}