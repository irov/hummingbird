#ifndef HB_RAND_H_
#define HB_RAND_H_

#include "hb_config/hb_config.h"

uint32_t hb_rand_seed( uint32_t _seed );
uint32_t hb_rand_time();
void hb_rand_hex_generate( const char * _vocabulary, hb_size_t _count, char * _hex, hb_size_t _size );

uint64_t hb_rand64( uint64_t * const _seed );

#define HB_SECURE_RAND_SECRET_SIZE 32

hb_result_t hb_secure_rand_initialize( const hb_byte_t _secret[HB_SECURE_RAND_SECRET_SIZE], uint64_t _time );
void hb_secure_rand_finalize( void );
void hb_secure_rand_bytes( void * _buffer, hb_size_t _size );

#endif
