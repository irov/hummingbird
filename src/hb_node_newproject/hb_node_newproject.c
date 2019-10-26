#include "hb_node_newproject.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_oid.h"

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

    //MessageBox( NULL, "Test", "Test", MB_OK );

    hb_log_initialize();
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

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

    hb_node_newproject_in_t in_data;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in_data, sizeof( in_data ), HB_NULLPTR ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    if( in_data.magic_number != hb_node_newproject_magic_number )
    {
        return EXIT_FAILURE;
    }

    if( in_data.version_number != hb_node_newproject_version_number )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_newproject", in_data.db_uri ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_collection_handle_t db_projects_handle;
    if( hb_db_get_collection( "hb", "hb_projects", &db_projects_handle ) == HB_FAILURE )
    {
        return EXIT_FAILURE;
    }

    hb_db_value_handle_t new_value[1];
    hb_db_make_int64_value( "script_revision", ~0U, 0, new_value + 0 );

    hb_oid_t project_oid;
    hb_db_new_document( &db_projects_handle, new_value, 1, project_oid );

    uint16_t pid = 0;
    uint32_t founds = 0;
    for( ; founds != 1; )
    {
        pid = (uint16_t)hb_rand_time();

        hb_db_value_handle_t handles[1];
        hb_db_make_int32_value( "pid", ~0U, pid, handles + 0 );

        hb_db_update_values( &db_projects_handle, project_oid, handles, 1 );
        
        hb_db_count_values( &db_projects_handle, handles, 1, &founds );
    }

    hb_db_finalize();

    hb_node_newproject_out_t out_data;
    out_data.magic_number = hb_node_newproject_magic_number;
    out_data.version_number = hb_node_newproject_version_number;
    out_data.pid = pid;

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, &out_data, sizeof( out_data ) );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}