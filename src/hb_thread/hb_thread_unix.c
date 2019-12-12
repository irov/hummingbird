#include "hb_thread.h"

#include "hb_log/hb_log.h"

#include <unistd.h>
#include <pthread.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_thread_proxy_t
{
    hb_thread_function_t function;
    void * ud;
}hb_thread_proxy_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_thread_handle_t
{
    pthread_t id;
    hb_thread_proxy_t * proxy;
}hb_thread_handle_t;
//////////////////////////////////////////////////////////////////////////
static void * __hb_thread_proxy( void * _ud )
{
    hb_thread_proxy_t * proxy = (hb_thread_proxy_t *)(_ud);

    (*proxy->function)(proxy->ud);

    pthread_exit( HB_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_thread_create( hb_thread_function_t _function, void * _ud, hb_thread_handle_t * _handle )
{
    hb_thread_proxy_t * proxy = HB_NEW( hb_thread_proxy_t );
    proxy->function = _function;
    proxy->ud = _ud;

    pthread_t id;
    pthread_create( &id, HB_NULLPTR, &__hb_thread_proxy, proxy );
    
    hb_thread_handle_t * handle = HB_NEW( hb_thread_handle_t );

    handle->id = id;
    handle->proxy = proxy;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_join( hb_thread_handle_t * _handle )
{
    pthread_t id = _handle->id;

    pthread_join( id, HB_NULLPTR );
}
//////////////////////////////////////////////////////////////////////////
void hb_thread_destroy( hb_thread_handle_t * _handle )
{
    pthread_t id = _handle->id;

    pthread_cancel( id );
    pthread_join( id, HB_NULLPTR );

    HB_DELETE( _handle->proxy );
    HB_DELETE( _handle );
}