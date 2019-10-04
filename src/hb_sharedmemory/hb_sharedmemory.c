#include "hb_sharedmemory.h"

#include "hb_config/hb_config.h"

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_create( const char * _name, size_t _size, hb_sharedmemory_handler_t * _handler )
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

    strcpy( _handler->name, _name );
    _handler->size = _size;
    _handler->carriage = 0;
    _handler->handler = (void *)hMapFile;
    _handler->buffer = (void *)pBuf;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_destroy( hb_sharedmemory_handler_t * _handler )
{
    LPVOID pBuf = (LPVOID)_handler->buffer;
    HANDLE hMapFile = (HANDLE)_handler->handler;    

    UnmapViewOfFile( pBuf );
    CloseHandle( hMapFile );

    _handler->size = 0;
    _handler->carriage = 0;
    _handler->handler = HB_NULLPTR;
    _handler->buffer = HB_NULLPTR;    

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_rewind( hb_sharedmemory_handler_t * _handler )
{
    _handler->carriage = 0;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_write( hb_sharedmemory_handler_t * _handler, const void * _buffer, size_t _size )
{
    if( _handler->carriage + 4 + _size > _handler->size )
    {
        return 0;
    }

    uint32_t u32_size = (uint32_t)_size;

    uint8_t * pBuf = (uint8_t *)_handler->buffer;

    CopyMemory( (PVOID)pBuf, &u32_size, sizeof( u32_size ) );

    _handler->carriage += 4;

    pBuf += _handler->carriage;

    CopyMemory( (PVOID)pBuf, _buffer, _size );

    _handler->carriage += _size;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_open( const char * _name, size_t _size, hb_sharedmemory_handler_t * _handler )
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

    strcpy( _handler->name, _name );
    _handler->size = _size;
    _handler->carriage = 0;
    _handler->handler = (void *)hMapFile;
    _handler->buffer = (void *)pBuf;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_sharedmemory_read( hb_sharedmemory_handler_t * _handler, void * _buffer, size_t _capacity, size_t * _size )
{
    if( _handler->carriage + 4 > _handler->size )
    {
        return 0;
    }

    uint8_t * pBuf = (uint8_t *)_handler->buffer;

    uint32_t u32_size;
    CopyMemory( &u32_size, pBuf, sizeof( u32_size ) );

    if( _capacity < u32_size )
    {
        return 0;
    }

    _handler->carriage += 4;

    if( _handler->carriage + u32_size > _handler->size )
    {
        return 0;
    }

    pBuf += _handler->carriage;

    CopyMemory( _buffer, pBuf, u32_size );

    _handler->carriage += u32_size;

    *_size = u32_size;

    return 1;
}