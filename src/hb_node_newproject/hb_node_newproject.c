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

    const char * db_uri;
    if( hb_getopt( _argc, _argv, "--db_uri", &db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_initialze( "hb_node_newproject", db_uri );

    //const char * sm_name;
    //if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == 0 )
    //{
    //    return 0;
    //}

    //hb_sharedmemory_handle_t sharedmemory_handle;
    //if( hb_sharedmemory_open( sm_name, 10240, &sharedmemory_handle ) == 0 )
    //{
    //    return 0;
    //}

    //size_t data_size;
    //char data[2048];
    //if( hb_sharedmemory_read( &sharedmemory_handle, data, 2048, &data_size ) == 0 )
    //{
    //    return 0;
    //}

    //hb_json_handle_t json_handle;
    //if( hb_json_create( data, data_size, &json_handle ) == 0 )
    //{
    //    return 0;
    //}

    hb_db_collection_handle_t db_projects_handle;
    hb_db_get_collection( "hb", "hb_projects", &db_projects_handle );

    uint8_t oid[12];
    hb_db_new_document( &db_projects_handle, oid );

    hb_db_value_handle_t new_value;
    hb_make_string_value( "test", ~0U, "test", ~0U, &new_value );

    hb_db_new_value( &db_projects_handle, oid, &new_value );

    hb_db_finalize();

    //hb_sharedmemory_rewind( &sharedmemory_handle );
    //hb_sharedmemory_write( &sharedmemory_handle, scriptr_result, scriptr_result_size );
    //hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}