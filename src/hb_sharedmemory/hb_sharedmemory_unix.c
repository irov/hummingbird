#include "hb_sharedmemory.h"

#include <sys/mman.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_create( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle )
{
    int protection = PROT_READ | PROT_WRITE;

    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    void * pBuf = mmap( HB_NULLPTR, _size, protection, visibility, -1, 0 );

    if( pBuf == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    strcpy( _handle->name, _name );
    _handle->size = _size;
    _handle->carriage = 0;
    _handle->handle = HB_NULLPTR;
    _handle->buffer = (void *)pBuf;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle )
{
    LPVOID pBuf = (LPVOID)_handle->buffer;
    HANDLE hMapFile = (HANDLE)_handle->handle;    

    UnmapViewOfFile( pBuf );
    CloseHandle( hMapFile );

    _handle->size = 0;
    _handle->carriage = 0;
    _handle->handle = HB_NULLPTR;
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
    CopyMemory( (PVOID)pBufSize, &u32_size, sizeof( uint32_t ) );

    _handle->carriage += sizeof( uint32_t );

    uint8_t * pBufData = (uint8_t *)_handle->buffer + _handle->carriage;
    CopyMemory( (PVOID)pBufData, _buffer, _size );

    _handle->carriage += _size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_open( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle )
{
    HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, _name );
    
    if( hMapFile == NULL )
    {
        return HB_FAILURE;
    }

    LPVOID pBuf = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _size );

    if( pBuf == NULL )
    {
        return HB_FAILURE;
    }

    strcpy( _handle->name, _name );
    _handle->size = _size;
    _handle->carriage = 0;
    _handle->handle = (void *)hMapFile;
    _handle->buffer = (void *)pBuf;

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
    CopyMemory( &u32_size, pBufSize, sizeof( uint32_t ) );

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

    CopyMemory( _buffer, pBufData, u32_size );

    _handle->carriage += u32_size;

    if( _size != HB_NULLPTR )
    {
        *_size = u32_size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////