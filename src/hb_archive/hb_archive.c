#include "hb_archive.h"

#include "hb_log/hb_log.h"

#include "lz4.h"
#include "lz4hc.h"

//////////////////////////////////////////////////////////////////////////
hb_size_t hb_archive_bound( hb_size_t _size )
{
    int32_t size = LZ4_compressBound( (int32_t)_size );

    return (hb_size_t)size;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_archive_compress( void * _buffer, hb_size_t _capacity, const void * _source, hb_size_t _size, hb_size_t * const _compressSize )
{
    char * dst_buffer = (char *)_buffer;
    const char * src_buffer = (const char *)_source;

    int32_t compress_method = LZ4HC_CLEVEL_MAX;

    int32_t compressSize = LZ4_compress_HC( src_buffer, dst_buffer, (int32_t)_size, (int32_t)_capacity, compress_method );

    if( compressSize < 0 )
    {
        HB_LOG_MESSAGE_ERROR( "archive", "invalid compress code [%d]"
            , compressSize
        );

        return HB_FAILURE;
    }

    *_compressSize = (hb_size_t)compressSize;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_archive_decompress( void * _buffer, hb_size_t _capacity, const void * _source, hb_size_t _size, hb_size_t * const _decompressSize )
{
    char * dst_buffer = (char *)_buffer;
    const char * src_buffer = (const char *)_source;

    int32_t decompressSize = LZ4_decompress_safe( src_buffer, dst_buffer, (int32_t)_size, (int32_t)_capacity );

    if( decompressSize < 0 )
    {
        HB_LOG_MESSAGE_ERROR( "archive", "invalid uncompress code [%d]"
            , decompressSize
        );

        return HB_FAILURE;
    }

    *_decompressSize = (hb_size_t)decompressSize;

    return HB_SUCCESSFUL;
}
