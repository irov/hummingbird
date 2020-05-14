#include "hb_storage.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_archive/hb_archive.h"
#include "hb_cache/hb_cache.h"
#include "hb_utils/hb_sha1.h"
#include "hb_utils/hb_base64.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_set( const hb_db_client_handle_t * _client, const void * _code, size_t _codesize, const char * _source, size_t _sourcesize, hb_sha1_t * _sha1 )
{
    size_t bound_size = hb_archive_bound( _codesize );

    if( bound_size >= HB_DATA_MAX_SIZE )
    {
        return HB_FAILURE;
    }

    size_t archive_script_code_size;
    hb_data_t archive_script_code_buffer;
    if( hb_archive_compress( archive_script_code_buffer, HB_DATA_MAX_SIZE, _code, _codesize, &archive_script_code_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_sha1( archive_script_code_buffer, archive_script_code_size, _sha1 );

    hb_db_collection_handle_t * db_collection_scripts;
    if( hb_db_get_collection( _client, "hb", "hb_scripts", &db_collection_scripts ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_db_upload_script( db_collection_scripts, *_sha1, archive_script_code_buffer, archive_script_code_size, _source, _sourcesize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_collection( db_collection_scripts );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_storage_get_code( const hb_db_client_handle_t * _client, const hb_sha1_t _sha1, void * _buffer, size_t _capacity, size_t * _size )
{
    hb_bool_t cache_available = hb_cache_available();

    if( cache_available == HB_TRUE )
    {
        size_t cache_data_size;
        hb_data_t cache_data;
        if( hb_cache_get_value( _sha1, 20, cache_data, HB_DATA_MAX_SIZE, &cache_data_size ) == HB_SUCCESSFUL )
        {
            if( hb_archive_decompress( _buffer, _capacity, cache_data, cache_data_size, _size ) == HB_SUCCESSFUL )
            {
                return HB_SUCCESSFUL;
            }
        }
    }

    hb_db_collection_handle_t * db_collection;
    if( hb_db_get_collection( _client, "hb", "hb_scripts", &db_collection ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_script_handle_t * db_script_data;
    if( hb_db_load_script( db_collection, _sha1, &db_script_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    size_t script_data_size;
    const hb_byte_t * script_data_buffer = hb_db_get_script_data( db_script_data, &script_data_size );

    if( cache_available == HB_TRUE )
    {
        if( hb_cache_set_value( _sha1, 20, script_data_buffer, script_data_size ) == HB_FAILURE )
        {
            HB_LOG_MESSAGE_ERROR( "storage", "invalid cache value ['%.20s']"
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