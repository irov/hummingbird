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

    hb_log_initialize();
    hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer );

    const char * db_uri;
    if( hb_getopt( _argc, _argv, "--db_uri", &db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_db_initialze( "hb_node_api", db_uri );

    hb_storage_initialize( "$user_id$", "hb", "hb_files" );
    hb_file_initialize( ".store/" );
    hb_script_initialize( 10240, 10240 );
    hb_script_user_initialize( "5d932e6820cdb53b7c26b73f", "hb", "hb_users" );

    FILE * f = fopen( "server.lua", "rb" );
    fseek( f, 0L, SEEK_END );
    long sz = ftell( f );
    rewind( f );

    char fbuf[2048];
    fread( fbuf, sz, 1, f );
    fclose( f );

    size_t code_size;
    uint8_t code_buffer[10240];
    if( hb_script_compiler( fbuf, sz, code_buffer, 10240, &code_size ) == 0 )
    {
        return 0;
    }

    uint8_t sha1[20];
    if( hb_storage_set( code_buffer, code_size, sha1 ) == 0 )
    {
        return EXIT_FAILURE;
    }

    const char * sm_name;
    if( hb_getopt( _argc, _argv, "--sm", &sm_name ) == 0 )
    {
        return 0;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 10240, &sharedmemory_handle ) == 0 )
    {
        return 0;
    }

    size_t data_size;
    char data[2048];
    if( hb_sharedmemory_read( &sharedmemory_handle, data, 2048, &data_size ) == 0 )
    {
        return 0;
    }

    hb_json_handle_t json_handle;
    if( hb_json_create( data, data_size, &json_handle ) == 0 )
    {
        return 0;
    }

    size_t script_cmd_size;
    const char * script_cmd;    
    if( hb_json_get_string( &json_handle, "f", &script_cmd, &script_cmd_size ) == 0 )
    {
        return 0;
    }
        
    size_t script_data_size;
    char script_data[10240];
    if( hb_json_dumpb_value( &json_handle, "d", script_data, 10240, &script_data_size ) == 0 )
    {
        return 0;
    }

    size_t fbufl;
    char fbufg[10240];
    if( hb_storage_get( sha1, fbufg, 10240, &fbufl ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_script_user_load( fbufg, fbufl ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t scriptr_result_size;
    char scriptr_result[2048];
    if( hb_script_user_call( script_cmd, script_cmd_size, script_data, script_data_size, scriptr_result, 2048, &scriptr_result_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_script_user_finalize();
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