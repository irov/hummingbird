#include "hb_rand.h"

#include "hb_time.h"
#include "hb_clock.h"

#include "hb_mutex/hb_mutex.h"

#include <assert.h>
#include <string.h>

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
static uint32_t s_secure_rand_key[8];
static uint64_t s_secure_rand_time;
static uint64_t s_secure_rand_block_counter;
static hb_mutex_handle_t * s_secure_rand_mutex;
//////////////////////////////////////////////////////////////////////////
static uint32_t __hb_secure_rand_load32( const hb_byte_t * _value )
{
    return (uint32_t)_value[0] |
        (uint32_t)_value[1] << 8 |
        (uint32_t)_value[2] << 16 |
        (uint32_t)_value[3] << 24;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_secure_rand_store32( hb_byte_t * _value, uint32_t _word )
{
    _value[0] = (hb_byte_t)(_word >> 0);
    _value[1] = (hb_byte_t)(_word >> 8);
    _value[2] = (hb_byte_t)(_word >> 16);
    _value[3] = (hb_byte_t)(_word >> 24);
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __hb_secure_rand_rotate_left( uint32_t _value, uint32_t _shift )
{
    return (_value << _shift) | (_value >> (32 - _shift));
}
//////////////////////////////////////////////////////////////////////////
static void __hb_secure_rand_quarter_round( uint32_t * _a, uint32_t * _b, uint32_t * _c, uint32_t * _d )
{
    *_a += *_b;
    *_d ^= *_a;
    *_d = __hb_secure_rand_rotate_left( *_d, 16 );

    *_c += *_d;
    *_b ^= *_c;
    *_b = __hb_secure_rand_rotate_left( *_b, 12 );

    *_a += *_b;
    *_d ^= *_a;
    *_d = __hb_secure_rand_rotate_left( *_d, 8 );

    *_c += *_d;
    *_b ^= *_c;
    *_b = __hb_secure_rand_rotate_left( *_b, 7 );
}
//////////////////////////////////////////////////////////////////////////
static void __hb_secure_rand_chacha20_block( uint64_t _counter, hb_byte_t _block[64] )
{
    uint32_t state[16] = {
        0x61707865U, 0x3320646eU, 0x79622d32U, 0x6b206574U,
        s_secure_rand_key[0], s_secure_rand_key[1], s_secure_rand_key[2], s_secure_rand_key[3],
        s_secure_rand_key[4], s_secure_rand_key[5], s_secure_rand_key[6], s_secure_rand_key[7],
        (uint32_t)_counter, (uint32_t)(_counter >> 32),
        (uint32_t)s_secure_rand_time, (uint32_t)(s_secure_rand_time >> 32)
    };

    uint32_t working[16];
    memcpy( working, state, sizeof( working ) );

    for( uint32_t round = 0; round != 10; ++round )
    {
        __hb_secure_rand_quarter_round( &working[0], &working[4], &working[8], &working[12] );
        __hb_secure_rand_quarter_round( &working[1], &working[5], &working[9], &working[13] );
        __hb_secure_rand_quarter_round( &working[2], &working[6], &working[10], &working[14] );
        __hb_secure_rand_quarter_round( &working[3], &working[7], &working[11], &working[15] );
        __hb_secure_rand_quarter_round( &working[0], &working[5], &working[10], &working[15] );
        __hb_secure_rand_quarter_round( &working[1], &working[6], &working[11], &working[12] );
        __hb_secure_rand_quarter_round( &working[2], &working[7], &working[8], &working[13] );
        __hb_secure_rand_quarter_round( &working[3], &working[4], &working[9], &working[14] );
    }

    for( uint32_t index = 0; index != 16; ++index )
    {
        __hb_secure_rand_store32( _block + index * 4, working[index] + state[index] );
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_secure_rand_initialize( const hb_byte_t _secret[HB_SECURE_RAND_SECRET_SIZE], uint64_t _time )
{
    assert( _secret != HB_NULLPTR );

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    for( uint32_t index = 0; index != 8; ++index )
    {
        s_secure_rand_key[index] = __hb_secure_rand_load32( _secret + index * 4 );
    }

    s_secure_rand_time = _time;
    s_secure_rand_block_counter = 1;
    s_secure_rand_mutex = mutex;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_secure_rand_finalize( void )
{
    hb_mutex_destroy( s_secure_rand_mutex );
    s_secure_rand_mutex = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
void hb_secure_rand_bytes( void * _buffer, hb_size_t _size )
{
    assert( _buffer != HB_NULLPTR || _size == 0 );

    uint64_t block_count = _size / 64 + (_size % 64 != 0 ? 1 : 0);

    hb_mutex_lock( s_secure_rand_mutex );

    uint64_t first_counter = s_secure_rand_block_counter;
    s_secure_rand_block_counter += block_count;

    hb_mutex_unlock( s_secure_rand_mutex );

    hb_byte_t * output = (hb_byte_t *)_buffer;
    hb_byte_t block[64];
    hb_size_t remaining = _size;
    uint64_t counter = first_counter;
    while( remaining != 0 )
    {
        __hb_secure_rand_chacha20_block( counter++, block );

        hb_size_t block_size = HB_MIN( remaining, sizeof( block ) );
        memcpy( output, block, block_size );
        output += block_size;
        remaining -= block_size;
    }
}
//////////////////////////////////////////////////////////////////////////
