#ifndef HB_CACHE_H_
#define HB_CACHE_H_

#include "hb_config/hb_config.h"

int hb_cache_initialze( const char * _uri, uint32_t _port );
void hb_cache_finalize();

int hb_cache_set_binary_value( const char * _key, const void * _value, size_t _size );
int hb_cache_set_string_value( const char * _key, const char * _value, size_t _size );
int hb_cache_set_int64_value( const char * _key, int64_t _value );

int hb_cache_get_binary_value( const char * _key, void * _value, size_t _capacity, size_t * _size );
int hb_cache_get_string_value( const char * _key, char * _value, size_t _capacity, size_t * _size );
int hb_cache_get_int64_value( const char * _key, int64_t * _value );

#endif
