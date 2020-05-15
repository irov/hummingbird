#ifndef HB_POOL_H_
#define HB_POOL_H_

#include "hb_config/hb_config.h"

typedef struct hb_pool_handle_t hb_pool_handle_t;

typedef hb_result_t( *hb_pool_new_t )(void ** _ptr, void * _ud);
typedef void( *hb_pool_delete_t )(void * _ptr, void * _ud);

hb_result_t hb_pool_create( hb_pool_new_t _new, hb_pool_delete_t _delete, void * _ud, hb_pool_handle_t ** _handle );
void hb_pool_destroy( hb_pool_handle_t * _handle );

hb_result_t hb_pool_pop( hb_pool_handle_t * _handle, void ** _ptr );
hb_result_t hb_pool_push( hb_pool_handle_t * _handle, void * _ptr );

#endif
