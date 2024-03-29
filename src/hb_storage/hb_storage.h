#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"

hb_result_t hb_storage_set_code( const hb_db_collection_handle_t * _collection, const void * _code, hb_size_t _codesize, const char * _source, hb_size_t _sourcesize, hb_sha1_t * _sha1 );
hb_result_t hb_storage_get_code( const hb_cache_handle_t * _cache, const hb_db_collection_handle_t * _collection, const hb_sha1_t * _sha1, void * _buffer, hb_size_t _capacity, hb_size_t * _size );

#endif
