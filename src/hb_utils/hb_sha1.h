#ifndef HB_SHA1_H_
#define HB_SHA1_H_

#include "hb_config/hb_config.h"

void hb_sha1_copy( hb_sha1_t _dst, const hb_sha1_t _src );

void hb_sha1( const void * _buffer, const size_t _size, hb_sha1_t * _sha1 );

#endif
