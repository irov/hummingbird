#ifndef HB_BASE64_H_
#define HB_BASE64_H_

#include "hb_config/hb_config.h"

size_t hb_base64_encode_size( size_t _size );
size_t hb_base64_decode_size( size_t _size );

hb_result_t hb_base64_encode( const void * _binary, size_t _size, char * _base64, size_t _capacity, size_t * _outsize );
hb_result_t hb_base64_decode( const char * _base64, size_t _base64size, void * _binary, size_t _capacity, size_t * _outsize );

#endif
