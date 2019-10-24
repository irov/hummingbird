#include "hb_token.h"

#include "hb_rand.h"

#include <string.h>

void hb_token_copy( hb_token_t _dst, const hb_token_t _src )
{
    memcpy( _dst, _src, 12 );
}

void hb_token_generate( uint64_t _index, hb_token_t _token )
{
    uint32_t index_low = (uint32_t)(_index >> 0);
    uint32_t index_hight = (uint32_t)(_index >> 32);

    uint32_t rand_index_low = hb_rand_seed( index_low );
    uint32_t rand_index_hight = hb_rand_seed( index_hight );
    uint32_t rand_time = hb_rand_time();

    uint32_t rand_index_hight_salt = hb_rand_seed( index_low );
    rand_index_hight ^= rand_index_hight_salt;

    memcpy( _token + 0, &rand_index_low, 4 );
    memcpy( _token + 4, &rand_time, 4 );
    memcpy( _token + 8, &rand_index_hight, 4 );
}