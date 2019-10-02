#ifndef HB_SHA1_H_
#define HB_SHA1_H_

#include <stdint.h>

void hb_sha1( const void * _buffer, const size_t _size, uint8_t * _sha1 );
void hb_sha1_hex( const uint8_t * _sha1, char * _hex );

#endif
