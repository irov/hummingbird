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
void hb_rand_hex_generate( const char * _vocabulary, hb_size_t _count, char * _hex, hb_size_t _size )
{
    uint32_t rand = hb_rand_time();

    for( hb_size_t index = 0; index != _size; ++index )
    {
        uint32_t element = rand % (uint32_t)_count;

        _hex[index] = _vocabulary[element];

        rand = hb_rand_seed( rand );
    }
}
//////////////////////////////////////////////////////////////////////////
static inline uint64_t __ror64( uint64_t v, uint64_t r )
{
    return (v >> r) | (v << (64 - r));
}
//////////////////////////////////////////////////////////////////////////
uint64_t hb_rand64( uint64_t * const _seed )
{
    const uint64_t prime = 0x9FB21C651E98DF25L;

    uint64_t v = *_seed;

    v ^= __ror64( v, 49 ) ^ __ror64( v, 24 );
    v *= prime;
    v ^= v >> 28;
    v *= prime;
    v ^= v >> 28;

    *_seed = v;

    return v;
}
//////////////////////////////////////////////////////////////////////////