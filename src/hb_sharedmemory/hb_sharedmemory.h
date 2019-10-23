#ifndef HB_SHAREDMEMORY_H_
#define HB_SHAREDMEMORY_H_

#include "hb_config/hb_config.h"

typedef struct hb_sharedmemory_handle_t
{
    char name[64];
    size_t size;
    size_t carriage;
    void * handle;
    void * buffer;

} hb_sharedmemory_handle_t;

hb_result_t hb_sharedmemory_create( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle );
hb_result_t hb_sharedmemory_open( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle );
hb_result_t hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle );

hb_result_t hb_sharedmemory_rewind( hb_sharedmemory_handle_t * _handle );
hb_result_t hb_sharedmemory_write( hb_sharedmemory_handle_t * _handle, const void * _buffer, size_t _size );
hb_result_t hb_sharedmemory_read( hb_sharedmemory_handle_t * _handle, void * _buffer, size_t _capacity, size_t * _size );

#endif
