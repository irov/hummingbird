#include "hb_mutex.h"

#include "hb_log/hb_log.h"

#include "hb_platform/hb_platform_windows.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_mutex_handle_t
{
    CRITICAL_SECTION section;
}hb_mutex_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_mutex_create( hb_mutex_handle_t ** _handle )
{
    hb_mutex_handle_t * handle = HB_NEW( hb_mutex_handle_t );

    InitializeCriticalSection( &handle->section );

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_destroy( hb_mutex_handle_t * _handle )
{
    DeleteCriticalSection( &_handle->section );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_mutex_try_lock( hb_mutex_handle_t * _handle )
{
    if( TryEnterCriticalSection( &_handle->section ) == FALSE )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_lock( hb_mutex_handle_t * _handle )
{
    EnterCriticalSection( &_handle->section );
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_unlock( hb_mutex_handle_t * _handle )
{
    LeaveCriticalSection( &_handle->section );
}
//////////////////////////////////////////////////////////////////////////