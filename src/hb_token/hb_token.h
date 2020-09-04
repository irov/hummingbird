#ifndef HB_TOKEN_H_
#define HB_TOKEN_H_

#include "hb_cache/hb_cache.h"

#include "hb_config/hb_config.h"

void hb_token_copy( hb_token_t * _dst, const hb_token_t * _src );
hb_result_t hb_token_base16_encode( const hb_token_t * _token, hb_token16_t * _token16 );
hb_result_t hb_token_base16_decode( const hb_token16_t * _token16, hb_token_t * _token );
hb_result_t hb_token_base16_decode_string( const char * _tokenstring, hb_token_t * _token );

hb_result_t hb_token_generate( const hb_cache_handle_t * _cache, const char _prefix[2], const void * _value, hb_size_t _size, uint32_t _expire, hb_token_t * _token );

#endif
