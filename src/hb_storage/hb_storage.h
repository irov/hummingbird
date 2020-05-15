#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"
#include "hb_cache/hb_cache.h"

hb_result_t hb_storage_set( const hb_db_client_handle_t * _db, const void * _code, size_t _codesize, const char * _source, size_t _sourcesize, hb_sha1_t * _sha1 );
hb_result_t hb_storage_get_code( const hb_cache_handle_t * _cache, const hb_db_client_handle_t * _db, const hb_sha1_t _sha1, void * _buffer, size_t _capacity, size_t * _size );

#endif
