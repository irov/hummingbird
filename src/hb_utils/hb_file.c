#include "hb_file.h"
#include "hb_log/hb_log.h"

#include <stdio.h>

hb_result_t hb_file_read( const char * _path, void * _buffer, hb_size_t _capacity, hb_size_t * _size )
{
    FILE * f = fopen( _path, "rb" );

    if( f == HB_NULLPTR )
    {
        HB_LOG_MESSAGE_CRITICAL( "file", "file '%s' not found"
            , _path
        );

        return HB_FAILURE;
    }

    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    if( sz > (long)_capacity )
    {
        HB_LOG_MESSAGE_CRITICAL( "file", "file '%s' very large [%d]"
            , _path
            , _capacity
        );

        fclose( f );

        return HB_FAILURE;
    }

    hb_size_t r = fread( _buffer, sz, 1, f );

    fclose( f );

    if( _size != HB_NULLPTR )
    {
        *_size = r;
    }

    return HB_SUCCESSFUL;
}