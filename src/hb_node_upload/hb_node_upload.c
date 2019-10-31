#include "hb_node_upload.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_memmem.h"

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

    //MessageBox( NULL, "Test", "Test", MB_OK );

    if( hb_log_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 65536, &sharedmemory_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_node_upload_in_t in_data;
    if( hb_node_read_in_data( &sharedmemory_handle, &in_data, sizeof( in_data ), hb_node_upload_magic_number, hb_node_upload_version_number ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    size_t code_size;
    hb_data_t code_buffer;
    if( hb_script_compiler( in_data.data, in_data.data_size, code_buffer, HB_DATA_MAX_SIZE, &code_size ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_upload", in_data.db_uri ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( hb_storage_initialize() == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_sha1_t sha1;
    if( hb_storage_set( code_buffer, code_size, sha1 ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    hb_db_value_handle_t project_handles[1];

    hb_db_make_int32_value( "pid", ~0U, in_data.pid, project_handles + 0 );

    hb_oid_t project_oid;
    hb_bool_t project_exist;
    if( hb_db_find_oid( &db_projects_handle, project_handles, 1, project_oid, &project_exist ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( project_exist == HB_FALSE )
    {
        return EXIT_FAILURE;
    }

    const char * db_projects_fields[] = { "script_revision" };

    hb_db_value_handle_t db_script_revision_handle[1];
    if( hb_db_get_values( &db_projects_handle, project_oid, db_projects_fields, 1, db_script_revision_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    int64_t script_revision = db_script_revision_handle[0].u.i64;
    
    hb_db_destroy_values( db_script_revision_handle, 1 );

    hb_db_collection_handle_t db_project_subversion_handler;
    if( hb_db_get_collection( "hb", "hb_project_subversion", &db_project_subversion_handler ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_oid_t project_subversion_oid;
    
    if( script_revision == 0 )
    {
        hb_db_value_handle_t handles[1];
        hb_db_make_binary_value( "sha1", ~0U, sha1, 20, handles + 0 );

        hb_db_new_document( &db_project_subversion_handler, handles, 1, project_subversion_oid );
    }
    else
    {
        hb_db_value_handle_t db_script_sha1_handle;
        hb_db_get_value( &db_projects_handle, project_oid, "script_sha1", &db_script_sha1_handle );

        hb_db_value_handle_t handles[2];
        hb_db_make_binary_value( "sha1", ~0U, sha1, 20, handles + 0 );
        hb_db_make_binary_value( "prev", ~0U, db_script_sha1_handle.u.binary.buffer, db_script_sha1_handle.u.binary.length, handles + 1 );

        hb_db_new_document( &db_project_subversion_handler, handles, 2, project_subversion_oid );

        hb_db_destroy_values( &db_script_sha1_handle, 1 );
    }

    hb_db_value_handle_t db_project_handles[3];
    hb_db_make_binary_value( "script_sha1", ~0U, sha1, 20, db_project_handles + 0 );
    hb_db_make_oid_value( "script_subversion", ~0U, project_subversion_oid, db_project_handles + 1 );
    hb_db_make_int64_value( "script_revision", ~0U, script_revision + 1, db_project_handles + 2 );
    

    hb_db_update_values( &db_projects_handle, project_oid, db_project_handles, 3 );

    hb_storage_finalize();

    hb_db_destroy_collection( &db_projects_handle );
    hb_db_destroy_collection( &db_project_subversion_handler );

    hb_db_finalize();

    hb_node_upload_out_t out_data;
    out_data.revision = script_revision;

    hb_node_write_out_data( &sharedmemory_handle, &out_data, sizeof( out_data ), hb_node_upload_magic_number, hb_node_upload_version_number );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}