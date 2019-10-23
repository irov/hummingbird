#ifndef HB_BASE16_H_
#define HB_BASE16_H_

#include "hb_config/hb_config.h"

size_t hb_base16_encode_size( size_t _size );
size_t hb_base16_decode_size( size_t _size );

int hb_base16_encode( const void * _binary, size_t _size, char * _base16, size_t _capacity, size_t * _outsize );
int hb_base16_decode( const char * _base16, size_t _base16size, void * _binary, size_t _capacity, size_t * _outsize );

#endif
