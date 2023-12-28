#ifndef HB_RAND_H_
#define HB_RAND_H_

#include "hb_config/hb_config.h"

uint32_t hb_rand_seed( uint32_t _seed );
uint32_t hb_rand_time();
void hb_rand_hex_generate( const char * _vocabulary, hb_size_t _count, char * _hex, hb_size_t _size );

uint64_t hb_rand64( uint64_t * const _seed );

#endif