#ifndef HB_TOKEN_H_
#define HB_TOKEN_H_

#include "hb_config/hb_config.h"

void hb_token_copy( hb_token_t _dst, const hb_token_t _src );
void hb_token_generate( uint64_t _index, hb_token_t _token );

#endif