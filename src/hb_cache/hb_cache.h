#ifndef HB_CACHE_H_
#define HB_CACHE_H_

#include "hb_config/hb_config.h"

typedef struct hb_cache_handle_t hb_cache_handle_t;

hb_result_t hb_cache_create( const char * _uri, uint16_t _port, uint32_t _timeout, hb_cache_handle_t ** _handle );
void hb_cache_destroy( hb_cache_handle_t * _handle );

hb_result_t hb_cache_set_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, const void * _value, size_t _size );
hb_result_t hb_cache_get_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, void * _value, size_t _capacity, size_t * _size );
hb_result_t hb_cache_incrby_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint64_t _increment, uint64_t * _value );
hb_result_t hb_cache_expire_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint32_t _seconds );
hb_result_t hb_cache_zadd( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, const void * _value, size_t _valuesize, uint32_t _score );
hb_result_t hb_cache_zrevrange( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint32_t _begin, uint32_t _end, const void ** _value, size_t * _count );

#endif
