#include "hb_node_newproject.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
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

    hb_log_initialize();
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == 0 )
    {
        return 0;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 65536, &sharedmemory_handle ) == 0 )
    {
        return 0;
    }

    hb_node_newproject_in_t in_data;
    if( hb_sharedmemory_read( &sharedmemory_handle, &in_data, sizeof( in_data ), HB_NULLPTR ) == 0 )
    {
        return 0;
    }

    hb_db_initialze( "hb_node_newproject", in_data.db_uri );

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    hb_db_value_handle_t new_value[1];
    hb_db_make_int64_value( "script_revision", ~0U, 0, new_value + 0 );

    uint8_t oid[12];
    hb_db_new_document( &db_projects_handle, new_value, 1, oid );

    hb_db_finalize();

    hb_node_newproject_out_t out_data;
    memcpy( out_data.puid, oid, 12 );

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, &out_data, sizeof( out_data ) );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}