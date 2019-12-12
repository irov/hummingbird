#include "hb_sharedmemory.h"

#include <sys/shm.h> 

#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_sharedmemory_handle_t
{
    int id;
    size_t size;
    size_t carriage;
    void * buffer;
    hb_bool_t create;

} hb_sharedmemory_handle_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_create( size_t _size, hb_sharedmemory_handle_t ** _handle )
{
    int shmid = shmget( IPC_PRIVATE, 1024, IPC_CREAT | IPC_EXCL );

    if( shmid == -1 )
    {
        return HB_FAILURE;
    }

    void * pBuf = (char*)shmat( shmid, 0, 0 ); 

    if( pBuf == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    hb_sharedmemory_handle_t * handle = HB_NEW( hb_sharedmemory_handle_t );
    
    handle->id = shmid;
    handle->size = _size;
    handle->carriage = 0;
    handle->buffer = pBuf;
    handle->create = HB_TRUE;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle )
{
    void * pBuf = _handle->buffer;

    shmdt( pBuf );

    if( _handle->create == HB_TRUE )
    {
        shmctl( _handle->id, IPC_RMID, 0 );
    }

    _handle->size = 0;
    _handle->carriage = 0;
    _handle->buffer = HB_NULLPTR;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_rewind( hb_sharedmemory_handle_t * _handle )
{
    _handle->carriage = 0;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_write( hb_sharedmemory_handle_t * _handle, const void * _buffer, size_t _size )
{
    if( _handle->carriage + sizeof( uint32_t ) + _size > _handle->size )
    {
        return HB_FAILURE;
    }

    uint32_t u32_size = (uint32_t)_size;

    uint8_t * pBufSize = (uint8_t *)_handle->buffer + _handle->carriage;
    memcpy( pBufSize, &u32_size, sizeof( uint32_t ) );

    _handle->carriage += sizeof( uint32_t );

    uint8_t * pBufData = (uint8_t *)_handle->buffer + _handle->carriage;
    memcpy( pBufData, _buffer, _size );

    _handle->carriage += _size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_open( uint32_t _id, size_t _size, hb_sharedmemory_handle_t ** _handle )
{
    void * buffer = shmat( _id, 0, 0 );
    
    if( buffer == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    hb_sharedmemory_handle_t * handle = HB_NEW( hb_sharedmemory_handle_t );

    handle->id = _id;
    handle->size = _size;
    handle->carriage = 0;
    handle->buffer = buffer;
    handle->create = HB_FALSE;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_read( hb_sharedmemory_handle_t * _handle, void * _buffer, size_t _capacity, size_t * _size )
{
    if( _handle->carriage + sizeof( uint32_t ) > _handle->size )
    {
        return HB_FAILURE;
    }

    uint8_t * pBufSize = (uint8_t *)_handle->buffer + _handle->carriage;

    uint32_t u32_size;
    memcpy( &u32_size, pBufSize, sizeof( uint32_t ) );

    if( _capacity < u32_size )
    {
        return HB_FAILURE;
    }

    _handle->carriage += sizeof( uint32_t );

    if( _handle->carriage + u32_size > _handle->size )
    {
        return HB_FAILURE;
    }

    uint8_t * pBufData = (uint8_t *)_handle->buffer + _handle->carriage;

    memcpy( _buffer, pBufData, u32_size );

    _handle->carriage += u32_size;

    if( _size != HB_NULLPTR )
    {
        *_size = u32_size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////