#ifndef HB_ARRAY_H_
#define HB_ARRAY_H_

#include "hb_config/hb_config.h"

typedef struct hb_array_t hb_array_t;

hb_result_t hb_array_create( const void * _data, hb_size_t _size, hb_array_t ** _array );
void hb_array_destroy( hb_array_t * _array );

const void * hb_array_data( hb_array_t * _array, hb_size_t * _size );

#endif
