#include "hb_archive.h"

#include "hb_log/hb_log.h"

#include "lz4.h"
#include "lz4hc.h"

//////////////////////////////////////////////////////////////////////////
size_t hb_archive_bound( size_t _size )
{
    int32_t size = LZ4_compressBound( (int)_size );
    
    return (size_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_archive_compress( void * _buffer, size_t _capacity, const void * _source, size_t _size, size_t * _compressSize )
{
    char * dst_buffer = (char *)_buffer;
    const char * src_buffer = (const char *)_source;

    int compress_method = LZ4HC_CLEVEL_MAX;

    int compressSize = LZ4_compress_HC( src_buffer, dst_buffer, (int32_t)_size, (int32_t)_capacity, compress_method );

    if( compressSize < 0 )
    {
        hb_log_message( "archive", HB_LOG_ERROR, "invalid compress code [%d]"
            , compressSize
        );

        return HB_FAILURE;
    }

    *_compressSize = (size_t)compressSize;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_archive_decompress( void * _buffer, size_t _capacity, const void * _source, size_t _size, size_t * _decompressSize )
{
    char * dst_buffer = (char *)_buffer;
    const char * src_buffer = (const char *)_source;

    int decompressSize = LZ4_decompress_safe( src_buffer, dst_buffer, (int32_t)_size, (int32_t)_capacity );

    if( decompressSize < 0 )
    {
        hb_log_message( "archive", HB_LOG_ERROR, "invalid uncompress code [%d]"
            , decompressSize
        );

        return HB_FAILURE;
    }

    *_decompressSize = (size_t)decompressSize;

    return HB_SUCCESSFUL;
}
