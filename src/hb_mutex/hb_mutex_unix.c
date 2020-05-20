#include "hb_mutex.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"

#include <errno.h>
#include <pthread.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_mutex_handle_t
{
    pthread_mutex_t id;
}hb_mutex_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_mutex_create( hb_mutex_handle_t ** _handle )
{
    hb_mutex_handle_t * handle = HB_NEW( hb_mutex_handle_t );

    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

    if( pthread_mutex_init( &handle->id, &attr ) != 0 )
    {
        return HB_FAILURE;
    }

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_destroy( hb_mutex_handle_t * _handle )
{
    pthread_mutex_destroy( &_handle->id );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_mutex_try_lock( hb_mutex_handle_t * _handle )
{
    if( pthread_mutex_trylock( &_handle->id ) == EBUSY )
    {
        return HB_FALSE;
    }

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_lock( hb_mutex_handle_t * _handle )
{
    pthread_mutex_lock( &_handle->id );
}
//////////////////////////////////////////////////////////////////////////
void hb_mutex_unlock( hb_mutex_handle_t * _handle )
{
    pthread_mutex_unlock( &_handle->id );
}
//////////////////////////////////////////////////////////////////////////