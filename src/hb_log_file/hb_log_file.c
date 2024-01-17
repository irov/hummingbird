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
static void __hb_log_file_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _file );
    HB_UNUSED( _line );

    hb_log_file_handle_t * handle = (hb_log_file_handle_t *)_ud;

    const char * ls = hb_log_level_string[_level];

    hb_date_t d;
    hb_date( &d );

#ifdef HB_DEBUG
    fprintf( handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s) [%s:%u]: %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _file, _line, _message );
#else
    fprintf( handle->f, "%s [%u.%u.%u][%u:%u:%u][%u] (%s): %s\n", ls, d.year, d.mon, d.mday, d.hour, d.min, d.sec, d.msec, _category, _message );
#endif

    fflush( handle->f );
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

    hb_log_file_handle_t * handle = HB_NEW( hb_log_file_handle_t );
    handle->f = f;

    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_file_observer, handle ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_file_finalize()
{
    void * ud;
    if( hb_log_remove_observer( &__hb_log_file_observer, &ud ) == HB_SUCCESSFUL )
    {
        hb_log_file_handle_t * handle = (hb_log_file_handle_t *)ud;

        fclose( handle->f );
        handle->f = HB_NULLPTR;

        HB_DELETE( handle );
    }
}
//////////////////////////////////////////////////////////////////////////