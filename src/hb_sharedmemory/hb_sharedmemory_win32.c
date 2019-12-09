#include "hb_sharedmemory.h"

#include "hb_platform/hb_platform_windows.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_sharedmemory_handle_t
{
    uint32_t id;
    char name[64];
    size_t size;
    size_t carriage;
    HANDLE hMapFile;
    LPVOID pBuf;

} hb_sharedmemory_handle_t;
//////////////////////////////////////////////////////////////////////////
static void __hb_write_name( char * _name, uint32_t _id )
{
    sprintf( _name, "hb_sm_%03u", _id );
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_create( uint32_t * _id, size_t _size, hb_sharedmemory_handle_t ** _handle )
{
    static uint32_t id = 0;

    ++id;

    char name[64];
    __hb_write_name( name, id );

    HANDLE hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, _size, name );

    if( hMapFile == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    LPVOID pBuf = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)_size );
    
    if( pBuf == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    hb_sharedmemory_handle_t * handle = HB_NEW( hb_sharedmemory_handle_t );

    handle->id = id;
    strcpy( handle->name, name );
    handle->size = _size;
    handle->carriage = 0;
    handle->hMapFile = hMapFile;
    handle->pBuf = pBuf;

    *_id = id;
    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_open( uint32_t _id, size_t _size, hb_sharedmemory_handle_t ** _handle )
{
    char name[64];
    __hb_write_name( name, _id );

    HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, name );

    if( hMapFile == NULL )
    {
        return HB_FAILURE;
    }

    LPVOID pBuf = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _size );

    if( pBuf == NULL )
    {
        return HB_FAILURE;
    }

    hb_sharedmemory_handle_t * handle = HB_NEW( hb_sharedmemory_handle_t );

    handle->id = _id;
    strcpy( handle->name, name );
    handle->size = _size;
    handle->carriage = 0;
    handle->hMapFile = hMapFile;
    handle->pBuf = pBuf;

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle )
{
    LPVOID pBuf = _handle->pBuf;
    HANDLE hMapFile = _handle->hMapFile;

    UnmapViewOfFile( pBuf );
    CloseHandle( hMapFile );

    _handle->size = 0;
    _handle->carriage = 0;
    _handle->hMapFile = INVALID_HANDLE_VALUE;
    _handle->pBuf = HB_NULLPTR;

    HB_DELETE( _handle );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_sharedmemory_get_id( hb_sharedmemory_handle_t * _handle )
{
    return _handle->id;
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

    PVOID pBufSize = (uint8_t *)_handle->pBuf + _handle->carriage;
    CopyMemory( pBufSize, &u32_size, sizeof( uint32_t ) );

    _handle->carriage += sizeof( uint32_t );

    PVOID pBufData = (uint8_t *)_handle->pBuf + _handle->carriage;
    CopyMemory( pBufData, _buffer, _size );

    _handle->carriage += _size;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_sharedmemory_read( hb_sharedmemory_handle_t * _handle, void * _buffer, size_t _capacity, size_t * _size )
{
    if( _handle->carriage + sizeof( uint32_t ) > _handle->size )
    {
        return HB_FAILURE;
    }

    PVOID pBufSize = (uint8_t *)_handle->pBuf + _handle->carriage;

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

    PVOID pBufData = (uint8_t *)_handle->pBuf + _handle->carriage;

    CopyMemory( _buffer, pBufData, u32_size );

    _handle->carriage += u32_size;

    if( _size != HB_NULLPTR )
    {
        *_size = u32_size;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////