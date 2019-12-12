#include "hb_sleep.h"

#include "hb_platform/hb_platform_windows.h"

void hb_sleep( uint32_t _ms )
{
    Sleep( _ms );
}