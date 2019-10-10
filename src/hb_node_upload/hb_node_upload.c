#include "hb_node_upload.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_file/hb_file.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_memmem.h"

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

    if( hb_log_initialize() == 0 )
    {
        return EXIT_FAILURE;
    }

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

    hb_node_upload_in_t in;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in, sizeof( hb_node_upload_in_t ), HB_NULLPTR ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t code_size;
    uint8_t code_buffer[10240];
    if( hb_script_compiler( in.data, in.data_size, code_buffer, 10240, &code_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_upload", in.db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_storage_initialize( "$user_id$", "hb", "hb_files" ) == 0 )
    {
        return EXIT_FAILURE;
    }

    uint8_t sha1[20];
    if( hb_storage_set( code_buffer, code_size, sha1 ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    hb_db_value_handle_t db_script_version_handle;
    hb_db_get_value( &db_projects_handle, in.puid, "script_version", e_hb_db_int64, &db_script_version_handle );

    int64_t script_version = db_script_version_handle.value_int64;
    
    hb_db_value_destroy( &db_script_version_handle );

    hb_db_collection_handle_t db_project_scripts_handler;
    if( hb_db_get_collection( "hb", "hb_project_scripts", &db_project_scripts_handler ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( script_version == 0 )
    {
        uint8_t oid[12];
        hb_db_new_document( &db_project_scripts_handler, oid );

        hb_db_value_handle_t handles[1];
        hb_make_buffer_value( "sha1", ~0U, sha1, 20, handles + 0 );

        hb_db_new_values( &db_project_scripts_handler, oid, handles, 1 );
    }
    else
    {

    }


    hb_storage_finalize();

    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, "OK", 2 );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}