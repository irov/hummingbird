#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

int hb_storage_initialize( const char * _user, const char * _db, const char * _collection, const char * _folder );
void hb_storage_finalize();

int hb_storage_set( const void * _data, size_t _size );
int hb_storage_get( void * _data, size_t _capacity );

#endif
