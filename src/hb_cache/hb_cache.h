#ifndef HB_CACHE_H_
#define HB_CACHE_H_

#include "hb_config/hb_config.h"

typedef struct hb_cache_handle_t hb_cache_handle_t;

hb_result_t hb_cache_create( const char * _uri, uint16_t _port, uint32_t _timeout, hb_cache_handle_t ** _handle );
void hb_cache_destroy( hb_cache_handle_t * _handle );

hb_result_t hb_cache_set_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, const void * _value, size_t _valuesize );
hb_result_t hb_cache_get_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, void * _value, size_t _capacity, size_t * _size );
hb_result_t hb_cache_incrby_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint64_t _increment, uint64_t * _value );
hb_result_t hb_cache_expire_value( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint32_t _seconds );
hb_result_t hb_cache_get_token( const hb_cache_handle_t * _cache, const char * _token, uint32_t _seconds, void * _value, size_t _capacity, size_t * _size );
hb_result_t hb_cache_zadd( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, const void * _value, size_t _valuesize, uint32_t _score );

typedef enum hb_cache_value_type_e
{
    e_hb_cache_integer,
    e_hb_cache_double,
    e_hb_cache_string,
} hb_cache_value_type_e;

typedef struct hb_cache_value_t
{
    hb_cache_value_type_e type;
    int64_t integer;
    double real;
    char string[128];
} hb_cache_value_t;

hb_result_t hb_cache_zrevrange( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, uint32_t _begin, uint32_t _end, hb_cache_value_t * _values, uint32_t * _count );
hb_result_t hb_cache_zrevrank( const hb_cache_handle_t * _cache, const void * _key, size_t _keysize, const void * _value, size_t _valuesize, uint32_t * _score, hb_bool_t * _exist );

#endif
