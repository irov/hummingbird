#include "hb_log_stderr.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static void __hb_log_observer( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message, void * _ud )
{
    HB_UNUSED( _ud );

    const char * ls = hb_log_level_string[_level];

    switch( _level )
    {
    case HB_LOG_INFO:
    case HB_LOG_WARNING:
        {
            fprintf( stdout, "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
        }break;
    case HB_LOG_ERROR:
    case HB_LOG_CRITICAL:
        {
            fprintf( stderr, "%s [%s:%u] %s: %s\n", ls, _file, _line, _category, _message );
        }break;
    default:
        break;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_stderr_initialize()
{
    if( hb_log_add_observer( HB_NULLPTR, HB_LOG_ALL, &__hb_log_observer, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_stderr_finalize()
{
    fflush( stdout );
    fflush( stderr );

    hb_log_remove_observer( &__hb_log_observer, HB_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////