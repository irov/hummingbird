#ifndef HB_HASHTABLE_H
#define HB_HASHTABLE_H

#include "hb_config/hb_config.h"

typedef struct hb_hashtable_t hb_hashtable_t;

hb_result_t hb_hashtable_create( size_t _capacity, hb_hashtable_t ** _ht );
void hb_hashtable_emplace( hb_hashtable_t * _ht, const void * _key, size_t _size, void * _element );
void * hb_hashtable_change( hb_hashtable_t * _ht, const void * _key, size_t _size, void * _element );
void * hb_hashtable_erase( hb_hashtable_t * _ht, const void * _key, size_t _size );
void * hb_hashtable_find( hb_hashtable_t * _ht, const void * _key, size_t _size );
void hb_hashtable_reserve( hb_hashtable_t * _ht, size_t _capacity );
hb_bool_t hb_hashtable_empty( hb_hashtable_t * _ht );
size_t hb_hashtable_size( hb_hashtable_t * _ht );
void hb_hashtable_clear( hb_hashtable_t * _ht );

#endif