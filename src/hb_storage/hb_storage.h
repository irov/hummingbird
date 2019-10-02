#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

int hb_storage_initialize( const char * _user, const char * _db, const char * _collection );
void hb_storage_finalize();

int hb_storage_set( const void * _data, size_t _size, uint8_t * _sha1 );
int hb_storage_get( const uint8_t * _sha1, void * _data, size_t _capacity, size_t * _size );

#endif
