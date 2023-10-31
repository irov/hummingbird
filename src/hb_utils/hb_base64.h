#ifndef HB_BASE64_H_
#define HB_BASE64_H_

#include "hb_config/hb_config.h"

hb_size_t hb_base64_encode_size( hb_size_t _size );
hb_size_t hb_base64_decode_size( const char * _base64, hb_size_t _size );

hb_result_t hb_base64_encode( const void * _binary, hb_size_t _size, char * _base64, hb_size_t _capacity, hb_size_t * _outsize );
hb_result_t hb_base64_decode( const char * _base64, hb_size_t _size, void * _binary, hb_size_t _capacity, hb_size_t * _outsize );
hb_result_t hb_base64_decode_string( const char * _base64, hb_size_t _size, char * _buffer, hb_size_t _capacity, hb_size_t * _outsize );

#endif
