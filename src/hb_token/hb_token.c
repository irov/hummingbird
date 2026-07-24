#include "hb_token.h"

#include "hb_cache/hb_cache.h"
#include "hb_log/hb_log.h"

#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <assert.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
void hb_token_copy( hb_token_t * _dst, const hb_token_t * _src )
{
    memcpy( _dst->value, _src->value, sizeof( hb_token_t ) );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_token_base16_encode( const hb_token_t * _token, hb_token16_t * _token16 )
{
    hb_result_t result = hb_base16_encode( _token->value, sizeof( hb_token_t ), _token16->value, sizeof( hb_token16_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_token_base16_decode( const hb_token16_t * _token16, hb_token_t * _token )
{
    hb_result_t result = hb_base16_decode( _token16->value, sizeof( hb_token16_t ), _token->value, sizeof( hb_token_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_token_base16_decode_string( const char * _tokenstring, hb_token_t * _token )
{
    assert( _tokenstring != HB_NULLPTR && strlen( _tokenstring ) == sizeof( hb_token16_t ) );

    hb_result_t result = hb_base16_decode( _tokenstring, sizeof( hb_token16_t ), _token->value, sizeof( hb_token_t ), HB_NULLPTR );

    return result;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_token_generate( const hb_cache_handle_t * _cache, const char _prefix[2], const void * _value, hb_size_t _size, uint32_t _expire, hb_token_t * _token )
{
    assert( _cache != HB_NULLPTR && _prefix != HB_NULLPTR && _value != HB_NULLPTR && _token != HB_NULLPTR );

    memcpy( _token->value + 0, _prefix, 2 );

    hb_secure_rand_bytes( _token->value + 2, sizeof( _token->value ) - 2 );

    if( hb_cache_set_value( _cache, _token->value, sizeof( hb_token_t ), _value, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_cache_expire_value( _cache, _token->value, sizeof( hb_token_t ), _expire, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
