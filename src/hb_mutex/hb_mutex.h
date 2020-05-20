#ifndef HB_MUTEX_H_
#define HB_MUTEX_H_

#include "hb_config/hb_config.h"

typedef struct hb_mutex_handle_t hb_mutex_handle_t;

hb_result_t hb_mutex_create( hb_mutex_handle_t ** _handle );
void hb_mutex_destroy( hb_mutex_handle_t * _handle );

hb_bool_t hb_mutex_try_lock( hb_mutex_handle_t * _handle );
void hb_mutex_lock( hb_mutex_handle_t * _handle );
void hb_mutex_unlock( hb_mutex_handle_t * _handle );

#endif
