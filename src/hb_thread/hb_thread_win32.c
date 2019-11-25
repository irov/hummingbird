#include "hb_thread.h"

#include "hb_log/hb_log.h"

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <process.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_thread_create( hb_thread_function_t _function, void * _ud, hb_thread_handle_t * _handle )
{
    uint32_t id;
    uintptr_t handle = _beginthreadex( HB_NULLPTR, 0, _function, _ud, 0, &id );
    
    _handle->id = id;
    _handle->handle = (void *)handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_join( hb_thread_handle_t * _handle )
{
    HANDLE handle = (HANDLE)_handle->handle;

    WaitForSingleObject( handle, INFINITE );
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_destroy( hb_thread_handle_t * _handle )
{
    HANDLE handle = (HANDLE)_handle->handle;

    CloseHandle( handle );
}