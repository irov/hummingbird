#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

#include "hb_db/hb_db.h"

int hb_storage_initialize( const hb_db_collection_handle_t * _collection );
void hb_storage_finalize();

int hb_storage_set( const void * _data, size_t _size, uint8_t * _sha1 );
int hb_storage_get( const uint8_t * _sha1, void * _data, size_t _capacity, size_t * _size );

#endif
