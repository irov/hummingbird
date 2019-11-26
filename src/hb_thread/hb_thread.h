#ifndef HB_STORAGE_H_
#define HB_STORAGE_H_

#include "hb_config/hb_config.h"

typedef struct hb_thread_handle_t hb_thread_handle_t;

typedef uint32_t( __stdcall * hb_thread_function_t )(void * _ud);

hb_result_t hb_thread_create( hb_thread_function_t _function, void * _ud, hb_thread_handle_t ** _handle );
void hb_thread_join( hb_thread_handle_t * _handle );
void hb_thread_destroy( hb_thread_handle_t * _handle );

#endif
