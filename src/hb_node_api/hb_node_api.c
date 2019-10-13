#include "hb_node_api.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_file/hb_file.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, int _level, const char * _message )
{    
    const char * ls[] = {"info", "warning", "error", "critical"};

    printf( "[%s] %s: %s\n", _category, ls[_level], _message );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    HB_UNUSED( _argc );
    HB_UNUSED( _argv );

    MessageBox( NULL, "Test", "Test", MB_OK );

    hb_log_initialize();
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 65536, &sharedmemory_handle ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_node_api_in_t in_data;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in_data, sizeof( in_data ), HB_NULLPTR ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_initialze( "hb_node_api", in_data.db_uri );

    hb_db_collection_handle_t db_token_handle;
    hb_db_get_collection( "hb", "hb_token", &db_token_handle );
    
    const char * db_token_fields[] = { "uuid", "puid" };

    hb_db_value_handle_t db_uid_handle[2];
    if( hb_db_get_values( &db_token_handle, in_data.token, db_token_fields, 2, db_uid_handle ) == 0 )
    {
        return EXIT_FAILURE;
    }

    uint8_t uuid[12];
    memcpy( uuid, db_uid_handle[0].value_oid, 12 );

    uint8_t puid[12];
    memcpy( puid, db_uid_handle[1].value_oid, 12 );
    
    hb_db_destroy_values( db_uid_handle, 2 );

    hb_db_collection_handle_t db_user_data_handle;
    hb_db_get_collection( "hb", "hb_users_data", &db_user_data_handle );

    hb_db_collection_handle_t db_project_data_handle;
    hb_db_get_collection( "hb", "hb_projects_data", &db_project_data_handle );

    if( hb_script_initialize( 10240, 10240, &db_user_data_handle, &db_project_data_handle, uuid, puid ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_files_handle;
    hb_db_get_collection( "hb", "hb_files", &db_files_handle );

    if( hb_storage_initialize( &db_files_handle ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_file_initialize( ".store/" ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    const char * db_projects_fields[] = { "script_sha1" };

    hb_db_value_handle_t db_script_sha1_handle[1];
    if( hb_db_get_values( &db_projects_handle, puid, db_projects_fields, 1, db_script_sha1_handle ) == 0 )
    {
        return EXIT_FAILURE;
    }

    uint8_t script_sha1[20];
    memcpy( script_sha1, db_script_sha1_handle[0].value_binary, 20 );

    hb_db_destroy_values( db_script_sha1_handle, 1 );

    size_t json_data_size;
    char json_data[10240];
    if( hb_storage_get( script_sha1, json_data, 10240, &json_data_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_script_load( json_data, json_data_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_json_handle_t json_handle;
    if( hb_json_create( in_data.data, in_data.data_size, &json_handle ) == 0 )
    {
        return 0;
    }

    size_t script_data_size;
    char script_data[10240];
    if( hb_json_dumpb( &json_handle, script_data, 10240, &script_data_size ) == 0 )
    {
        return 0;
    }

    size_t scriptr_result_size;
    char scriptr_result[2048];
    if( hb_script_call( in_data.method, script_data, script_data_size, scriptr_result, 2048, &scriptr_result_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_script_finalize();
    hb_file_finialize();
    hb_storage_finalize();
    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, scriptr_result, scriptr_result_size );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}