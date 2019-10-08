#include "hb_sharedmemory.h"

#include "hb_config/hb_config.h"

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_create( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle )
{
    HANDLE hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, _size, _name );

    if( hMapFile == NULL )
    {
        return 0;
    }

    LPVOID pBuf = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)_size );
    
    if( pBuf == NULL )
    {
        return 0;
    }

    strcpy( _handle->name, _name );
    _handle->size = _size;
    _handle->carriage = 0;
    _handle->handle = (void *)hMapFile;
    _handle->buffer = (void *)pBuf;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_destroy( hb_sharedmemory_handle_t * _handle )
{
    LPVOID pBuf = (LPVOID)_handle->buffer;
    HANDLE hMapFile = (HANDLE)_handle->handle;    

    UnmapViewOfFile( pBuf );
    CloseHandle( hMapFile );

    _handle->size = 0;
    _handle->carriage = 0;
    _handle->handle = HB_NULLPTR;
    _handle->buffer = HB_NULLPTR;    

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_rewind( hb_sharedmemory_handle_t * _handle )
{
    _handle->carriage = 0;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_write( hb_sharedmemory_handle_t * _handle, const void * _buffer, size_t _size )
{
    if( _handle->carriage + 4 + _size > _handle->size )
    {
        return 0;
    }

    uint32_t u32_size = (uint32_t)_size;

    uint8_t * pBufSize = (uint8_t *)_handle->buffer + _handle->carriage;
    CopyMemory( (PVOID)pBufSize, &u32_size, sizeof( u32_size ) );

    _handle->carriage += 4;

    uint8_t * pBufData = (uint8_t *)_handle->buffer + _handle->carriage;
    CopyMemory( (PVOID)pBufData, _buffer, _size );

    _handle->carriage += _size;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_open( const char * _name, size_t _size, hb_sharedmemory_handle_t * _handle )
{
    HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, _name );
    
    if( hMapFile == NULL )
    {
        return 0;
    }

    LPVOID pBuf = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _size );

    if( pBuf == NULL )
    {
        return 0;
    }

    strcpy( _handle->name, _name );
    _handle->size = _size;
    _handle->carriage = 0;
    _handle->handle = (void *)hMapFile;
    _handle->buffer = (void *)pBuf;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_read( hb_sharedmemory_handle_t * _handle, void * _buffer, size_t _capacity, size_t * _size )
{
    if( _handle->carriage + 4 > _handle->size )
    {
        return 0;
    }

    uint8_t * pBufSize = (uint8_t *)_handle->buffer + _handle->carriage;

    uint32_t u32_size;
    CopyMemory( &u32_size, pBufSize, sizeof( u32_size ) );

    if( _capacity < u32_size )
    {
        return 0;
    }

    _handle->carriage += 4;

    if( _handle->carriage + u32_size > _handle->size )
    {
        return 0;
    }

    uint8_t * pBufData = (uint8_t *)_handle->buffer + _handle->carriage;

    CopyMemory( _buffer, pBufData, u32_size );

    _handle->carriage += u32_size;

    if( _size != HB_NULLPTR )
    {
        *_size = u32_size;
    }

    return 1;
}