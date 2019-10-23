#include "hb_rand.h"

#include "hb_time.h"

uint32_t hb_rand( uint32_t _seed )
{
    uint32_t r = _seed * 1103515245U + 12345U;

    return r;
}

uint32_t hb_rand_time()
{
    hb_time_t time;
    hb_time( &time );

    uint32_t r = hb_rand( (uint32_t)time );

    return r;
}