#include "hb_multipart.h"

#include "hb_utils/hb_memmem.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
int hb_multipart_parse( const void * _boundary, size_t _boundarysize, multipart_params_desc_t * _params, size_t _capacity, const void * _buffer, size_t _buffersize, uint32_t * _count )
{
    uint32_t count = 0;

    size_t offset = 0;
    const void * boundary_iterator_begin = hb_memmeme( _buffer, _buffersize, _boundary, _boundarysize, &offset );

    while( boundary_iterator_begin != HB_NULLPTR )
    {
        if( count == _capacity )
        {
            return 1;
        }

        const void * boundary_iterator_end = hb_memmem( boundary_iterator_begin, _buffersize - offset, _boundary, _boundarysize, &offset );

        if( boundary_iterator_end == HB_NULLPTR )
        {
            return 1;
        }

        const char data_name_mask[] = "Content-Disposition: form-data; name=\"";
        const char * data_name = strstr( (const char *)boundary_iterator_begin, data_name_mask );
        data_name += sizeof( data_name_mask ) - 1;

        const char * data_end = strchr( data_name, '"' );
        size_t key_size = data_end - data_name;

        const char data_value_mask[] = "\r\n\r\n";
        const char * begin_data = strstr( data_end + 1, data_value_mask );
        begin_data += sizeof( data_value_mask ) - 1;
        HB_UNUSED( begin_data );

        multipart_params_desc_t * desc = _params + count;
        desc->key = data_name;
        desc->key_size = key_size;
        desc->memory_begin = (const void *)begin_data;

        size_t data_size = hb_memsize( begin_data, boundary_iterator_end );
        desc->memory_end = (const void *)(begin_data + data_size);

        ++count;

        boundary_iterator_begin = hb_memadvance( boundary_iterator_end, _boundarysize );
    }

    *_count = count;

    return 1;
}