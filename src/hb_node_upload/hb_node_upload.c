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
#include "hb_utils/hb_multipart.h"
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

    const char * db_uri;
    if( hb_getopt( _argc, _argv, "--db_uri", &db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_sharedmemory_handle_t sharedmemory_handle;
    if( hb_sharedmemory_open( sm_name, 10240, &sharedmemory_handle ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t sm_boundary_size;
    char sm_boundary[2048];
    if( hb_sharedmemory_read( &sharedmemory_handle, sm_boundary, 2048, &sm_boundary_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t sm_data_size;
    uint8_t sm_data[10240];
    if( hb_sharedmemory_read( &sharedmemory_handle, sm_data, 2048, &sm_data_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    uint32_t multipart_params_count;
    multipart_params_handle_t multipart_params[8];
    if( hb_multipart_parse( sm_boundary, sm_boundary_size, multipart_params, 8, sm_data, sm_data_size, &multipart_params_count ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t params_data_size;
    const void * params_data;
    if( hb_multipart_get_value( multipart_params, multipart_params_count, "data", &params_data, &params_data_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    size_t code_size;
    uint8_t code_buffer[10240];
    if( hb_script_compiler( params_data, params_data_size, code_buffer, 10240, &code_size ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_db_initialze( "hb_node_upload", db_uri ) == 0 )
    {
        return EXIT_FAILURE;
    }

    if( hb_storage_initialize( "$user_id$", "hb_storage", "hb_files" ) == 0 )
    {
        return EXIT_FAILURE;
    }

    uint8_t sha1[20];
    if( hb_storage_set( code_buffer, code_size, sha1 ) == 0 )
    {
        return EXIT_FAILURE;
    }

    hb_storage_finalize();

    hb_db_finalize();

    hb_sharedmemory_rewind( &sharedmemory_handle );
    hb_sharedmemory_write( &sharedmemory_handle, "OK", 2 );
    hb_sharedmemory_destroy( &sharedmemory_handle );

    hb_log_finalize();

    return EXIT_SUCCESS;
}