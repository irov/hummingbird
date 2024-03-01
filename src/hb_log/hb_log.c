#include "hb_log.h"

#include "hb_memory/hb_memory.h"
#include "hb_mutex/hb_mutex.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef HB_LOG_MAX_OBSERVER
#define HB_LOG_MAX_OBSERVER 16
#endif

#ifndef HB_LOG_MAX_CATEGORY_SIZE
#define HB_LOG_MAX_CATEGORY_SIZE 31
#endif

#ifndef HB_LOG_MAX_MESSAGE_SIZE
#define HB_LOG_MAX_MESSAGE_SIZE 4096
#endif

static const char * hb_log_level_string[] = {"all", "debug", "info", "warning", "error", "critical"};

//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_service_observer_desc_t
{
    hb_log_level_t level;
    char category[HB_LOG_MAX_CATEGORY_SIZE + 1];
    hb_log_observer_t observer;
    void * ud;
}hb_log_service_observer_desc_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_log_service_handle_t
{
    hb_log_level_t verbose_level;

    uint32_t observer_count;
    hb_log_service_observer_desc_t observers[HB_LOG_MAX_OBSERVER];

    hb_mutex_handle_t * mutex;
}hb_log_service_handle_t;
//////////////////////////////////////////////////////////////////////////
static hb_log_service_handle_t * g_log_service_handle = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_initialize()
{
    hb_log_service_handle_t * handle = HB_NEW( hb_log_service_handle_t );

    handle->verbose_level = HB_LOG_INFO;
    handle->observer_count = 0;

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->mutex = mutex;

    g_log_service_handle = handle;
    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_finalize()
{
    hb_mutex_destroy( g_log_service_handle->mutex );

    HB_DELETE( g_log_service_handle );
    g_log_service_handle = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
const char * hb_log_level_stringize( hb_log_level_t _level )
{
    const char * level_string = hb_log_level_string[_level];

    return level_string;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_level_parse( const char * _string, hb_log_level_t * const _level )
{
    for( hb_log_level_t level = 0; level != HB_LOG_CRITICAL + 1; ++level )
    {
        const char * level_string = hb_log_level_stringize( level );

        if( strcmp( level_string, _string ) != 0 )
        {
            continue;
        }

        *_level = level;

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
void hb_log_set_verbose_level( hb_log_level_t _level )
{
    g_log_service_handle->verbose_level = _level;
}
//////////////////////////////////////////////////////////////////////////
hb_log_level_t hb_log_get_verbose_level()
{
    return g_log_service_handle->verbose_level;
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_log_check_verbose_level( hb_log_level_t _level )
{
    return g_log_service_handle->verbose_level <= _level;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_add_observer( const char * _category, hb_log_level_t _level, hb_log_observer_t _observer, void * _ud )
{
    if( g_log_service_handle->observer_count == HB_LOG_MAX_OBSERVER )
    {
        return HB_FAILURE;
    }

    hb_log_service_observer_desc_t desc;
    desc.level = _level;

    if( _category != HB_NULLPTR )
    {
        strncpy( desc.category, _category, HB_LOG_MAX_CATEGORY_SIZE );
    }
    else
    {
        desc.category[0] = '\0';
    }

    desc.observer = _observer;
    desc.ud = _ud;

    g_log_service_handle->observers[g_log_service_handle->observer_count++] = desc;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_log_remove_observer( hb_log_observer_t _observer, void ** _ud )
{
    for( int32_t i = 0; i != HB_LOG_MAX_OBSERVER; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service_handle->observers + i;

        if( desc->observer != _observer )
        {
            continue;
        }

        if( _ud != HB_NULLPTR )
        {
            *_ud = desc->ud;
        }

        g_log_service_handle->observers[i] = g_log_service_handle->observers[g_log_service_handle->observer_count - 1];

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
static void __hb_log_message_args( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _message )
{
    uint32_t count = g_log_service_handle->observer_count;

    for( uint32_t i = 0; i != count; ++i )
    {
        hb_log_service_observer_desc_t * desc = g_log_service_handle->observers + i;

        if( desc->level > _level )
        {
            continue;
        }

        if( desc->category[0] != '\0' && strcmp( desc->category, _category ) != 0 )
        {
            continue;
        }

        (*desc->observer)(_category, _level, _file, _line, _message, desc->ud);
    }
}
//////////////////////////////////////////////////////////////////////////
void hb_log_message( const char * _category, hb_log_level_t _level, const char * _file, uint32_t _line, const char * _format, ... )
{
    va_list args;
    va_start( args, _format );

    char message[HB_LOG_MAX_MESSAGE_SIZE] = {'\0'};
    int32_t n = vsnprintf( message, HB_LOG_MAX_MESSAGE_SIZE, _format, args );

    va_end( args );

    if( n <= 0 )
    {
        return;
    }

    hb_mutex_lock( g_log_service_handle->mutex );

    __hb_log_message_args( _category, _level, _file, _line, message );

    hb_mutex_unlock( g_log_service_handle->mutex );
}
//////////////////////////////////////////////////////////////////////////