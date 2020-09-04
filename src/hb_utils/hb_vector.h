#ifndef HB_VECTOR_H_
#define HB_VECTOR_H_

#include "hb_config/hb_config.h"

typedef struct hb_vector_t hb_vector_t;

hb_result_t hb_vector_create( hb_size_t _elemsize, uint32_t _count, hb_vector_t ** _vector );
void hb_vector_destroy( hb_vector_t * _vector );

uint32_t hb_vector_count( const hb_vector_t * _vector );
hb_result_t hb_vector_resize( hb_vector_t * _vector, hb_size_t _count );

hb_result_t hb_vector_set( hb_vector_t * _vector, uint32_t _index, const void * _element );
hb_result_t hb_vector_get( const hb_vector_t * _vector, uint32_t _index, void ** _element );

#endif
