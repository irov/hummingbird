#include "hb_log.h"

#include "hb_config/hb_config.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef HB_LOG_MAX_OBSERVER
#define HB_LOG_MAX_OBSERVER 16
#endif

#ifndef HB_LOG_MAX_MESSAGE_SIZE
#define HB_LOG_MAX_MESSAGE_SIZE 2048
#endif

//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_service_observer_desc_t
{
    int level;
    hb_log_observer_t observer;
}hb_log_service_observer_desc_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_service_t
{
    int observer_count;
    hb_log_service_observer_desc_t observers[HB_LOG_MAX_OBSERVER];
}hb_log_service_t;
//////////////////////////////////////////////////////////////////////////
hb_log_service_t * g_log_service = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
int hb_log_add_observer( int _level, hb_log_observer_t _observer )
{
    if( g_log_service == HB_NULLPTR )
    {
        g_log_service = HB_NEW( hb_log_service_t );

        g_log_service->observer_count = 0;
    }

    if( g_log_service->observer_count == HB_LOG_MAX_OBSERVER )
    {
        return 0;
    }

    hb_log_service_observer_desc_t desc;
    desc.level = _level;
    desc.observer = _observer;

    g_log_service->observers[g_log_service->observer_count++] = desc;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_log_remove_observer( hb_log_observer_t _observer )
{
    if( g_log_service == HB_NULLPTR )
    {
        return 0;
    }

    for( int i = 0; i != HB_LOG_MAX_OBSERVER; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service->observers + i;

        if( desc->observer != _observer )
        {
            continue;
        }

        g_log_service->observers[i] = g_log_service->observers[g_log_service->observer_count - 1];

        return 1;
    }

    return 0;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_log_message_args( int _level, const char * _message )
{
    int count = g_log_service->observer_count;

    for( int i = 0; i != count; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service->observers + i;

        (*desc->observer)(_level, _message);
    }
}
//////////////////////////////////////////////////////////////////////////
void hb_log_message( int _level, const char * _format, ... )
{
    if( g_log_service == HB_NULLPTR )
    {
        return;
    }

    va_list args;
    va_start( args, _format );

    char message[HB_LOG_MAX_MESSAGE_SIZE];
    
    if( vsprintf( message, _format, args ) > 0 )
    {
        __hb_log_message_args( _level, message );
    }

    va_end( args );
}
//////////////////////////////////////////////////////////////////////////