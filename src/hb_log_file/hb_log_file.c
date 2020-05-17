#include "hb_log_file.h"

#include "hb_memory/hb_memory.h"
#include "hb_utils/hb_date.h"

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
static void __hb_log_file_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );
    HB_UNUSED( _file );
    HB_UNUSED( _line );

    const char * ls = hb_log_level_string[_level];

    hb_date_t d;
    hb_date( &d );

#ifdef HB_DEBUG
    fprintf( g_log_file_handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s) [%s:%u]: %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _file, _line, _message );
#else
    fprintf( g_log_file_handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s): %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _message );
#endif

    fflush( g_log_file_handle->f );
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

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_file_observer, HB_NULLPTR ) == HB_FAILURE )
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
        hb_log_remove_observer( &__hb_log_file_observer, HB_NULLPTR );

        fclose( g_log_file_handle->f );
        g_log_file_handle->f = HB_NULLPTR;

        HB_DELETE( g_log_file_handle );
        g_log_file_handle = HB_NULLPTR;
    }
}