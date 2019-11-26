#include "hb_thread.h"

#include "hb_log/hb_log.h"

#include "hb_platform/hb_platform_windows.h"

#include <process.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_thread_handle_t
{
    uint32_t id;
    HANDLE handle;
}hb_thread_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_thread_create( hb_thread_function_t _function, void * _ud, hb_thread_handle_t ** _handle )
{
    uint32_t id;
    uintptr_t th = _beginthreadex( HB_NULLPTR, 0, _function, _ud, 0, &id );
    
    hb_thread_handle_t * handle = HB_NEW( hb_thread_handle_t );

    handle->id = id;
    handle->handle = (HANDLE)th;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_join( hb_thread_handle_t * _handle )
{
    HANDLE handle = _handle->handle;

    WaitForSingleObject( handle, INFINITE );
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_destroy( hb_thread_handle_t * _handle )
{
    HANDLE handle = _handle->handle;

    CloseHandle( handle );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////