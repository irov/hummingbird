#ifndef HB_MEMMEM_H_
#define HB_MEMMEM_H_

#include "hb_config/hb_config.h"

size_t hb_memsize( const void * _begin, const void * _end );
const void * hb_memadvance( const void * _buffer, size_t _offset );
const void * hb_memmem( const void * _buffer, size_t _size, const void * _subbuffer, size_t _subsize, size_t * _offset );
const void * hb_memmeme( const void * _buffer, size_t _size, const void * _subbuffer, size_t _subsize, size_t * _offset );

#endif