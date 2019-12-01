#include "hb_rand.h"

#include "hb_time.h"
#include "hb_clock.h"

//////////////////////////////////////////////////////////////////////////
uint32_t hb_rand_seed( uint32_t _seed )
{
    uint32_t r = _seed * 1103515245U + 12345U;

    return r;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_rand_time()
{
    hb_clock_t msec;
    hb_clock_msec( &msec );

    hb_time_t time;
    hb_time( &time );

    uint32_t seed = (uint32_t)(time * 1000 + msec);

    uint32_t r = hb_rand_seed( seed );

    return r;
}
//////////////////////////////////////////////////////////////////////////
void hb_rand_hex_generate( const char * _vocabulary, size_t _count, char * _hex, size_t _size )
{
    uint32_t rand = hb_rand_time();

    for( size_t index = 0; index != _size; ++index )
    {
        uint32_t element = rand % (uint32_t)_count;

        _hex[index] = _vocabulary[element];

        rand = hb_rand_seed( rand );
    }
}