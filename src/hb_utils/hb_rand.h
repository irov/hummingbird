#ifndef HB_RAND_H_
#define HB_RAND_H_

#include "hb_config/hb_config.h"

uint32_t hb_rand_seed( uint32_t _seed );
uint32_t hb_rand_time();
void hb_rand_hex_generate( const char * _vocabulary, size_t _count, char * _hex, size_t _size );

#endif