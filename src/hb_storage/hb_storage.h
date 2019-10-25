#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

hb_result_t hb_storage_initialize();
void hb_storage_finalize();

hb_result_t hb_storage_set( const void * _data, size_t _size, hb_sha1_t _sha1 );
hb_result_t hb_storage_get( const hb_sha1_t _sha1, void * _data, size_t _capacity, size_t * _size );

#endif
