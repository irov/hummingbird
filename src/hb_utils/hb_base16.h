#ifndef HB_BASE16_H_
#define HB_BASE16_H_

#include "hb_config/hb_config.h"

hb_size_t hb_base16_encode_size( hb_size_t _size );
hb_size_t hb_base16_decode_size( hb_size_t _size );

hb_result_t hb_base16_encode( const void * _binary, hb_size_t _size, char * _base16, hb_size_t _capacity, hb_size_t * _outsize );
hb_result_t hb_base16_decode( const char * _base16, hb_size_t _base16size, void * _binary, hb_size_t _capacity, hb_size_t * _outsize );

#endif
