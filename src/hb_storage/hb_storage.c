#include "hb_storage.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_archive/hb_archive.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_storage_settings_t
{
    hb_db_collection_handle_t * db_collection;
} hb_storage_settings_t;
//////////////////////////////////////////////////////////////////////////
static hb_storage_settings_t * g_storage_settings;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_initialize()
{
    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( "hb", "hb_scripts", &db_collection ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    g_storage_settings = HB_NEW( hb_storage_settings_t );

    g_storage_settings->db_collection = db_collection;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_storage_finalize()
{   
    if( g_storage_settings != HB_NULLPTR )
    {
        hb_db_destroy_collection( g_storage_settings->db_collection );

        HB_DELETE( g_storage_settings );
        g_storage_settings = HB_NULLPTR;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_set( const void * _code, size_t _codesize, const char * _source, size_t _sourcesize, hb_sha1_t _sha1 )
{
    size_t bound_size = hb_archive_bound( _codesize );

    if( bound_size >= HB_DATA_MAX_SIZE )
    {
        return HB_FAILURE;
    }

    size_t archive_script_code_size;
    uint8_t archive_script_code_buffer[HB_DATA_MAX_SIZE];
    if( hb_archive_compress( archive_script_code_buffer, HB_DATA_MAX_SIZE, _code, _codesize, &archive_script_code_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1( archive_script_code_buffer, archive_script_code_size, _sha1 );

    if( hb_db_upload_script( g_storage_settings->db_collection, _sha1, archive_script_code_buffer, archive_script_code_size, _source, _sourcesize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_get_code( const hb_sha1_t _sha1, void * _buffer, size_t _capacity, size_t * _size )
{
    hb_result_t cache_available = hb_cache_available();

    if( cache_available == HB_SUCCESSFUL )
    {
        size_t cache_data_size;
        uint8_t cache_data[HB_DATA_MAX_SIZE];
        if( hb_cache_get_value( _sha1, 20, cache_data, HB_DATA_MAX_SIZE, &cache_data_size ) == HB_SUCCESSFUL )
        {
            if( hb_archive_decompress( _buffer, _capacity, cache_data, cache_data_size, _size ) == HB_SUCCESSFUL )
            {
                return HB_SUCCESSFUL;
            }
        }
    }

    hb_db_script_handle_t * db_script_data;
    if( hb_db_load_script( g_storage_settings->db_collection, _sha1, &db_script_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    size_t script_data_size;
    const uint8_t * script_data_buffer = hb_db_get_script_data( db_script_data, &script_data_size );

    if( cache_available == HB_SUCCESSFUL )
    {
        if( hb_cache_set_value( _sha1, 20, script_data_buffer, script_data_size ) == HB_FAILURE )
        {
            hb_log_message( "storage", HB_LOG_ERROR, "invalid cache value ['%.20s']"
                , _sha1
            );
        }
    }

    if( hb_archive_decompress( _buffer, _capacity, script_data_buffer, script_data_size, _size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_close_script( db_script_data );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////