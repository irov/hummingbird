#ifndef HB_SHAREDMEMORY_H_
#define HB_SHAREDMEMORY_H_

#include "hb_config/hb_config.h"

typedef struct hb_sharedmemory_handler_t
{
    char name[64];
    size_t size;
    size_t carriage;
    void * handler;
    void * buffer;

} hb_sharedmemory_handler_t;

int hb_sharedmemory_create( const char * _name, size_t _size, hb_sharedmemory_handler_t * _handler );
int hb_sharedmemory_open( const char * _name, size_t _size, hb_sharedmemory_handler_t * _handler );
int hb_sharedmemory_destroy( hb_sharedmemory_handler_t * _handler );

int hb_sharedmemory_rewind( hb_sharedmemory_handler_t * _handler );
int hb_sharedmemory_write( hb_sharedmemory_handler_t * _handler, const void * _buffer, size_t _size );
int hb_sharedmemory_read( hb_sharedmemory_handler_t * _handler, void * _buffer, size_t _capacity, size_t * _size );

#endif
