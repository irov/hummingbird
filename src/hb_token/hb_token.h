#ifndef HB_TOKEN_H_
#define HB_TOKEN_H_

#include "hb_config/hb_config.h"

void hb_token_copy( hb_token_t _dst, const hb_token_t _src );

hb_result_t hb_token_generate( const void * _prefix, const void * _value, size_t _size, uint32_t _expire, hb_token_t _token );

#endif
