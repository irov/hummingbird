#include "hb_admin.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_script/hb_script.h"
#include "hb_script/hb_script_compiler.h"
#include "hb_storage/hb_storage.h"
#include "hb_sharedmemory/hb_sharedmemory.h"
#include "hb_file/hb_file.h"
#include "hb_utils/hb_getopt.h"
#include "hb_utils/hb_httpopt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handler_t sharedmemory_handler;
    if( hb_sharedmemory_open( sm_name, 10240, &sharedmemory_handler ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t sm_data_size;
    char sm_data[2048];
    if( hb_sharedmemory_read( &sharedmemory_handler, sm_data, 2048, &sm_data_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    const char * db_uri;
    if( hb_getopt( _argc, _argv, "--db", &db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_initialze( "admin", db_uri );

    const char * db_cmd;
    if( hb_getopt( _argc, _argv, "--cmd", &db_cmd ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( strcmp( db_cmd, "upload_script" ) == 0 )
    {
        hb_storage_initialize( "$user_id$", "hb_storage", "hb_files" );
        hb_script_initialize( 10240, 10240 );

        size_t code_size;
        uint8_t code_buffer[10240];
        if( hb_script_compiler( sm_data, sm_data_size, code_buffer, 10240, &code_size ) == 0 )
        {
            return EXIT_FAILURE;
        }

        uint8_t sha1[20];
        if( hb_storage_set( code_buffer, code_size, sha1 ) == 0 )
        {
            return EXIT_FAILURE;
        }

        hb_storage_finalize();
        hb_script_finalize();
    }

    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handler );
    hb_sharedmemory_write( &sharedmemory_handler, "OK", 2 );
    hb_sharedmemory_destroy( &sharedmemory_handler );

    hb_log_finalize();

    return EXIT_SUCCESS;
}