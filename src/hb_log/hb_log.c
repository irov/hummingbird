#include "hb_log.h"

#include "hb_config/hb_config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
    char category[32];
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
hb_result_t hb_log_initialize()
{
    g_log_service = HB_NEW( hb_log_service_t );
    g_log_service->observer_count = 0;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_finalize()
{
    if( g_log_service != HB_NULLPTR )
    {
        HB_DELETE( g_log_service );
        g_log_service = HB_NULLPTR;
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_add_observer( const char * _category, int _level, hb_log_observer_t _observer )
{
    if( g_log_service->observer_count == HB_LOG_MAX_OBSERVER )
    {
        return HB_FAILURE;
    }

    hb_log_service_observer_desc_t desc;
    desc.level = _level;

    if( _category != HB_NULLPTR )
    {
        strcpy( desc.category, _category );
    }
    else
    {
        desc.category[0] = '\0';
    }

    desc.observer = _observer;

    g_log_service->observers[g_log_service->observer_count++] = desc;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_remove_observer( hb_log_observer_t _observer )
{
    for( int i = 0; i != HB_LOG_MAX_OBSERVER; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service->observers + i;

        if( desc->observer != _observer )
        {
            continue;
        }

        g_log_service->observers[i] = g_log_service->observers[g_log_service->observer_count - 1];

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_log_message_args( const char * _category, int _level, const char * _message )
{
    int count = g_log_service->observer_count;

    for( int i = 0; i != count; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service->observers + i;

        if( desc->level > _level )
        {
            continue;
        }

        if( desc->category[0] != '\0' && strcmp( desc->category, _category ) != 0 )
        {
            continue;
        }

        (*desc->observer)(_category, _level, _message);
    }
}
//////////////////////////////////////////////////////////////////////////
void hb_log_message( const char * _category, int _level, const char * _format, ... )
{
    va_list args;
    va_start( args, _format );

    char message[HB_LOG_MAX_MESSAGE_SIZE];    
    int n = vsprintf( message, _format, args );

    if( n > 0 )
    {
        __hb_log_message_args( _category, _level, message );
    }

    va_end( args );
}
//////////////////////////////////////////////////////////////////////////