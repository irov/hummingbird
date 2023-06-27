#include "hb_file.h"
#include "hb_log/hb_log.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
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

    if( r != 1 )
    {
        HB_LOG_MESSAGE_CRITICAL( "file", "file '%s' invalid read [%zu bytes]"
            , _path
            , sz
        );

        return HB_FAILURE;
    }

    if( _size != HB_NULLPTR )
    {
        *_size = sz;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_file_read_text( const char * _path, char * _buffer, hb_size_t _capacity, hb_size_t * _size )
{
    hb_size_t size;
    if( hb_file_read( _path, _buffer, _capacity, &size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _buffer[size] = '\0';
    
    if( _size != HB_NULLPTR )
    {
        *_size = size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////