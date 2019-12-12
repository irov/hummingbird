#include "hb_sleep.h"

#include <unistd.h>

void hb_sleep( uint32_t _ms )
{
    usleep( _ms * 1000 );
}