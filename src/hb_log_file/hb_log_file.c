#include "hb_log_file.h"

#include "hb_utils/hb_time.h"

#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_file_handle_t
{
    FILE * f;
}hb_log_file_handle_t;
//////////////////////////////////////////////////////////////////////////
static hb_log_file_handle_t * g_log_file_handle = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
static void __hb_log_file_observer( const char * _category, hb_log_level_e _level, const char * _message )
{
    hb_time_t t;
    hb_time( &t );

    fprintf( g_log_file_handle->f, "{\"time\":%" SCNu64 ", \"category\":\"%s\", \"level\":%u, \"message\":\"%s\"}\r\n", t, _category, _level, _message );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_file_initialize( const char * _file )
{
    if( _file == HB_NULLPTR )
    {
        return HB_SUCCESSFUL;
    }

    FILE * f = fopen( _file, "wb" );

    if( f == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    g_log_file_handle = HB_NEW( hb_log_file_handle_t );
    g_log_file_handle->f = f;

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_file_observer ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_file_finalize()
{
    if( g_log_file_handle != HB_NULLPTR )
    {
        hb_log_remove_observer( &__hb_log_file_observer );

        fclose( g_log_file_handle->f );
        g_log_file_handle->f = HB_NULLPTR;

        HB_DELETE( g_log_file_handle );
        g_log_file_handle = HB_NULLPTR;
    }
}