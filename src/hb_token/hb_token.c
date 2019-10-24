#include "hb_token.h"

#include "hb_cache/hb_cache.h"
#include "hb_log/hb_log.h"

#include "hb_utils/hb_rand.h"

#include <string.h>

void hb_token_copy( hb_token_t _dst, const hb_token_t _src )
{
    memcpy( _dst, _src, 12 );
}

hb_result_t hb_token_generate( const void * _prefix, const void * _value, size_t _size, uint32_t _expire, hb_token_t _token )
{
    if( hb_cache_available() == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    uint32_t rand_time = hb_rand_time();
    uint32_t increment = rand_time & 0x0000ffff;

    uint64_t token_index;
    hb_cache_incrby_value( "global", "token_enumerator", ~0U, increment, &token_index );

    uint32_t index_low = (uint32_t)(token_index >> 0);
    uint32_t index_hight = (uint32_t)(token_index >> 32);

    uint32_t rand_index_low = hb_rand_seed( index_low );
    uint32_t rand_index_hight = hb_rand_seed( index_hight );

    uint32_t rand_index_hight_salt = hb_rand_seed( index_low );
    rand_index_hight ^= rand_index_hight_salt;

    memcpy( _token + 0, &rand_index_low, 4 );
    memcpy( _token + 4, &rand_time, 4 );
    memcpy( _token + 8, &rand_index_hight, 4 );

    hb_cache_set_value( _prefix, _token, sizeof( hb_token_t ), _value, _size );
    hb_cache_expire_value( _prefix, _token, sizeof( hb_token_t ), _expire );

    return HB_SUCCESSFUL;
}