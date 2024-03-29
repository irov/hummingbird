#include "hb_multipart.h"

#include "hb_utils/hb_memmem.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_multipart_parse( const void * _boundary, hb_size_t _boundarysize, const void * _buffer, hb_size_t _buffersize, multipart_params_handle_t * _params, hb_size_t _capacity, uint32_t * _count )
{
    uint32_t count = 0;

    hb_size_t offset = 0;
    const void * boundary_iterator_begin = hb_memmeme( _buffer, _buffersize, _boundary, _boundarysize, &offset );

    while( boundary_iterator_begin != HB_NULLPTR )
    {
        if( count == _capacity )
        {
            break;
        }

        const void * boundary_iterator_end = hb_memmem( boundary_iterator_begin, _buffersize - offset, _boundary, _boundarysize, &offset );

        if( boundary_iterator_end == HB_NULLPTR )
        {
            break;
        }

        const char data_name_mask[] = "Content-Disposition: form-data; name=\"";
        const char * data_name = strstr( (const char *)boundary_iterator_begin, data_name_mask );

        if( data_name == HB_NULLPTR )
        {
            return HB_FAILURE;
        }

        data_name += sizeof( data_name_mask ) - 1;

        const char * data_end = strchr( data_name, '"' );
        hb_size_t key_size = data_end - data_name;

        const char data_value_mask[] = "\r\n\r\n";
        const char * begin_data = strstr( data_end + 1, data_value_mask );
        begin_data += sizeof( data_value_mask ) - 1;
        HB_UNUSED( begin_data );

        multipart_params_handle_t * handle = _params + count;
        handle->key = data_name;
        handle->key_size = key_size;
        handle->data = begin_data;

        hb_size_t data_size = hb_memsize( begin_data, boundary_iterator_end );
        handle->data_size = data_size - 2;

        ++count;

        boundary_iterator_begin = hb_memadvance( boundary_iterator_end, _boundarysize );
    }

    *_count = count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_multipart_get_value( multipart_params_handle_t * _handles, uint32_t _count, const char * _key, const void ** _data, hb_size_t * _size )
{
    for( uint32_t index = 0; index != _count; ++index )
    {
        multipart_params_handle_t * handle = _handles + index;

        if( strncmp( _key, handle->key, handle->key_size ) != 0 )
        {
            continue;
        }

        *_data = handle->data;
        *_size = handle->data_size;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}