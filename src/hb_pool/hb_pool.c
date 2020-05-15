#include "hb_pool.h"

#include "hb_mutex/hb_mutex.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_pool_node_handle_t
{
    void * ptr;

    struct hb_pool_node_handle_t * next;
} hb_pool_node_handle_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_pool_handle_t
{
    hb_pool_node_handle_t * node_free;

    hb_mutex_handle_t * mutex;

    hb_pool_new_t f_new;
    hb_pool_delete_t f_delete;
    void * ud;
} hb_pool_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_pool_create( hb_pool_new_t _new, hb_pool_delete_t _delete, void * _ud, hb_pool_handle_t ** _handle )
{
    hb_pool_handle_t * handle = HB_NEW( hb_pool_handle_t );

    handle->node_free = HB_NULLPTR;

    hb_mutex_handle_t * mutex;
    if( hb_mutex_create( &mutex ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    handle->mutex = mutex;

    handle->f_new = _new;
    handle->f_delete = _delete;
    handle->ud = _ud;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_pool_destroy( hb_pool_handle_t * _handle )
{
    while( _handle->node_free != HB_NULLPTR )
    {
        hb_pool_node_handle_t * node_free = _handle->node_free;

        _handle->f_delete( node_free->ptr, _handle->ud );

        _handle->node_free = node_free->next;

        HB_DELETE( node_free );
    }

    hb_mutex_destroy( _handle->mutex );

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_pool_pop( hb_pool_handle_t * _handle, void ** _ptr )
{
    hb_mutex_lock( _handle->mutex );

    if( _handle->node_free != HB_NULLPTR )
    {
        hb_pool_node_handle_t * node_free = _handle->node_free;

        void * ptr = node_free->ptr;

        _handle->node_free = node_free->next;

        hb_mutex_unlock( _handle->mutex );

        *_ptr = ptr;

        HB_DELETE( node_free );

        return HB_SUCCESSFUL;
    }

    void * new_ptr;
    if( _handle->f_new( _handle->ud, &new_ptr ) == HB_FAILURE )
    {
        hb_mutex_unlock( _handle->mutex );

        return HB_FAILURE;
    }

    hb_pool_node_handle_t * node = HB_NEW( hb_pool_node_handle_t );

    node->ptr = new_ptr;

    node->next = _handle->node_free;
    _handle->node_free = node;

    hb_mutex_unlock( _handle->mutex );

    *_ptr = new_ptr;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_pool_push( hb_pool_handle_t * _handle, void * _ptr )
{
    hb_pool_node_handle_t * node = HB_NEW( hb_pool_node_handle_t );

    node->ptr = _ptr;

    hb_mutex_lock( _handle->mutex );

    node->next = _handle->node_free;
    _handle->node_free = node;

    hb_mutex_unlock( _handle->mutex );

    return HB_SUCCESSFUL;
}
