#include "hb_node_api.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_cache/hb_cache.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_e _level, const char * _message )
{    
    const char * ls = hb_log_level_string[_level];

    printf( "[%s] %s: %s\n", _category, ls, _message );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    MessageBox( NULL, "Test", "Test", MB_OK );

    hb_log_initialize();
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_node_open_sharedmemory( _argc, _argv, &sharedmemory_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_api_in_t in_data;
    if( hb_node_read_in_data( &sharedmemory_handle, &in_data, sizeof( in_data ), hb_node_api_magic_number, hb_node_api_version_number ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_cache_initialize( in_data.cache_uri, in_data.cache_port, 5 ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_api", in_data.db_uri ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_storage_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_token_handle_t token_handle;
    if( hb_cache_get_value( in_data.token, sizeof( hb_token_t ), &token_handle, sizeof( hb_token_handle_t ), HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_cache_expire_value( in_data.token, sizeof( hb_token_t ), 1800 ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_user_data_handle;
    if( hb_db_get_collection( "hb", "hb_users", &db_user_data_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_project_data_handle;
    if( hb_db_get_collection( "hb", "hb_projects", &db_project_data_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_script_initialize( HB_DATA_MAX_SIZE, HB_DATA_MAX_SIZE, &db_user_data_handle, &db_project_data_handle, token_handle.uoid, token_handle.poid ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    if( hb_db_get_collection( "hb", "hb_projects", &db_projects_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const char * db_projects_fields[] = { "script_sha1" };

    hb_db_value_handle_t db_script_sha1_handles[1];
    if( hb_db_get_values( &db_projects_handle, token_handle.poid, db_projects_fields, 1, db_script_sha1_handles ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sha1_t script_sha1;
    memcpy( script_sha1, db_script_sha1_handles[0].u.binary.buffer, sizeof( hb_sha1_t ) );

    hb_db_destroy_values( db_script_sha1_handles, 1 );

    size_t script_data_size;
    hb_data_t script_data;
    if( hb_storage_get( script_sha1, script_data, HB_DATA_MAX_SIZE, &script_data_size ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_script_load( script_data, script_data_size ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_api_out_t out_data;

    switch( in_data.category )
    {
    case e_hb_node_api:
        {
            if( hb_script_server_call( in_data.method, in_data.data, in_data.data_size, out_data.response_data, HB_GRID_REQUEST_DATA_MAX_SIZE, &out_data.response_size, &out_data.successful ) == HB_FAILURE )
            {
                return EXIT_FAILURE;
            }
        }break;
    case e_hb_node_event:
        {
            if( hb_script_event_call( in_data.method, in_data.data, in_data.data_size ) == HB_FAILURE )
            {
                return EXIT_FAILURE;
            }

            out_data.successful = HB_TRUE;
            out_data.response_size = 0;
        }
    }

    hb_script_finalize();
    hb_cache_finalize();
    hb_storage_finalize();
    hb_db_finalize();

    hb_node_write_out_data( &sharedmemory_handle, &out_data, sizeof( out_data ), hb_node_api_magic_number, hb_node_api_version_number );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}