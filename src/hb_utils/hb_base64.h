#ifndef HB_BASE64_H_
#define HB_BASE64_H_

#include "hb_config/hb_config.h"

int hb_base64_decode( const uint8_t * _data, size_t _datasize, char * _base64, size_t _capacity, size_t * _base64size );
int hb_base64_encode( const char * _base64, size_t _base64size, uint8_t * _data, size_t _capacity, size_t * _datasize );

#endif
