#include "hb_memory.h"

#include <malloc.h>

//////////////////////////////////////////////////////////////////////////
static hb_memory_alloc_t g_alloc = HB_NULLPTR;
static hb_memory_realloc_t g_realloc = HB_NULLPTR;
static hb_memory_free_t g_free = HB_NULLPTR;
static void * g_ud = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
void hb_memory_initialize( hb_memory_alloc_t _alloc, hb_memory_realloc_t _realloc, hb_memory_free_t _free, void * _ud )
{
    g_alloc = _alloc;
    g_realloc = _realloc;
    g_free = _free;
    g_ud = _ud;
}
//////////////////////////////////////////////////////////////////////////
void * hb_memory_alloc( size_t _size )
{
    void * ptr = g_alloc( _size, g_ud );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
void * hb_memory_realloc( void * _ptr, size_t _size )
{
    void * ptr = g_realloc( _ptr, _size, g_ud );

    return ptr;
}
//////////////////////////////////////////////////////////////////////////
void hb_memory_free( const void * _ptr )
{
    g_free( _ptr, g_ud );
}
//////////////////////////////////////////////////////////////////////////