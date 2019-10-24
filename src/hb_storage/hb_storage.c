#include "hb_storage.h"

#include "hb_log/hb_log.h"
#include "hb_archive/hb_archive.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_storage_settings_t
{
    hb_db_collection_handle_t db_collection;
} hb_storage_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_storage_settings_t * g_storage_settings;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_initialize( const hb_db_collection_handle_t * _collection )
{
    g_storage_settings = HB_NEW( hb_storage_settings_t );

    g_storage_settings->db_collection = *_collection;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_storage_finalize()
{   
    HB_DELETE( g_storage_settings );
    g_storage_settings = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_set( const void * _data, size_t _size, hb_sha1_t _sha1 )
{
    size_t bound_size = hb_archive_bound( _size );

    if( bound_size >= HB_DATA_MAX_SIZE )
    {
        return HB_FAILURE;
    }

    size_t compressSize;

    uint8_t buffer[HB_DATA_MAX_SIZE];
    if( hb_archive_compress( buffer, HB_DATA_MAX_SIZE, _data, _size, &compressSize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1( buffer, compressSize, _sha1 );

    if( hb_db_upload_file( &g_storage_settings->db_collection, _sha1, buffer, compressSize ) == HB_FAILURE )
    {
        return 0;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_get( const hb_sha1_t _sha1, void * _data, size_t _capacity, size_t * _size )
{
    hb_result_t cache_available = hb_cache_available();

    if( cache_available == HB_SUCCESSFUL )
    {
        size_t cache_data_size;
        uint8_t cache_data[HB_DATA_MAX_SIZE];
        if( hb_cache_get_value( "storage", _sha1, 20, cache_data, HB_DATA_MAX_SIZE, &cache_data_size ) == HB_SUCCESSFUL )
        {
            if( hb_archive_decompress( _data, _capacity, cache_data, cache_data_size, _size ) == HB_SUCCESSFUL )
            {
                return HB_SUCCESSFUL;
            }
        }
    }

    hb_db_file_handle_t db_file_handle;
    if( hb_db_load_file( &g_storage_settings->db_collection, _sha1, &db_file_handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( cache_available == HB_SUCCESSFUL )
    {
        if( hb_cache_set_value( "storage", _sha1, 20, db_file_handle.buffer, db_file_handle.length ) == HB_FAILURE )
        {
            hb_log_message( "storage", HB_LOG_ERROR, "invalid cache value ['%.20s']"
                , _sha1
            );
        }
    }

    if( hb_archive_decompress( _data, _capacity, db_file_handle.buffer, db_file_handle.length, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_close_file( &db_file_handle );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////