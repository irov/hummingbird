#ifndef HB_MEMMEM_H_
#define HB_MEMMEM_H_

#include "hb_config/hb_config.h"

hb_size_t hb_memsize( const void * _begin, const void * _end );
const void * hb_memadvance( const void * _buffer, hb_size_t _offset );
const void * hb_memmem( const void * _buffer, hb_size_t _size, const void * _subbuffer, hb_size_t _subsize, hb_size_t * _offset );
const void * hb_memmeme( const void * _buffer, hb_size_t _size, const void * _subbuffer, hb_size_t _subsize, hb_size_t * _offset );

#endif