#ifndef HB_SHAREDMEMORY_H_
#define HB_SHAREDMEMORY_H_

#include "hb_config/hb_config.h"

typedef struct hb_sharedmemory_handle_t hb_sharedmemory_handle_t;

hb_result_t hb_sharedmemory_create( size_t _size, hb_sharedmemory_handle_t ** _handle );
hb_result_t hb_sharedmemory_open( uint32_t _id, size_t _size, hb_sharedmemory_handle_t ** _handle );
hb_result_t hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle );

uint32_t hb_sharedmemory_get_id( hb_sharedmemory_handle_t * _handle );

hb_result_t hb_sharedmemory_rewind( hb_sharedmemory_handle_t * _handle );
hb_result_t hb_sharedmemory_write( hb_sharedmemory_handle_t * _handle, const void * _buffer, size_t _size );
hb_result_t hb_sharedmemory_read( hb_sharedmemory_handle_t * _handle, void * _buffer, size_t _capacity, size_t * _size );

#endif
