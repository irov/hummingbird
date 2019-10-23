#ifndef HB_CACHE_H_
#define HB_CACHE_H_

#include "hb_config/hb_config.h"

hb_result_t hb_cache_available();

hb_result_t hb_cache_initialize( const char * _uri, uint32_t _port );
void hb_cache_finalize();

hb_result_t hb_cache_set_value( const void * _key, size_t _keysize, const void * _value, size_t _size );
hb_result_t hb_cache_get_value( const void * _key, size_t _keysize, void * _value, size_t _capacity, size_t * _size );
hb_result_t hb_cache_expire_value( const void * _key, size_t _keysize, uint32_t _seconds );

#endif