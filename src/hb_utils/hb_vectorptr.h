#ifndef HB_VECTORPTR_H_
#define HB_VECTORPTR_H_

#include "hb_config/hb_config.h"

typedef struct hb_vectorptr_t hb_vectorptr_t;

hb_result_t hb_vectorptr_create( uint32_t _count, hb_vectorptr_t ** _vector );
void hb_vectorptr_destroy( hb_vectorptr_t * _vector );

uint32_t hb_vectorptr_count( const hb_vectorptr_t * _vector );
hb_result_t hb_vectorptr_resize( hb_vectorptr_t * _vector, size_t _count );

hb_result_t hb_vectorptr_set( hb_vectorptr_t * _vector, uint32_t _index, void * _ptr );
hb_result_t hb_vectorptr_get( const hb_vectorptr_t * _vector, uint32_t _index, void ** _ptr );

typedef void(*hb_vectorptr_visit_t)(uint32_t _index, void * _ptr, void * _ud);

void hb_vectorptr_visit( const hb_vectorptr_t * _vector, hb_vectorptr_visit_t _visitor, void * _ud );

typedef hb_bool_t( *hb_vectorptr_find_t )(void * _ptr, const void * _ud);

void * hb_vectorptr_find( const hb_vectorptr_t * _vector, hb_vectorptr_find_t _find, const void * _ud );

#endif
