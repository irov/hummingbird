#include "hb_vector.h"

#include "hb_memory/hb_memory.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_vector_t
{
    void * data;
    size_t elemsize;
    uint32_t count;
} hb_vector_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vector_create( size_t _elemsize, uint32_t _count, hb_vector_t ** _vector )
{
    if( _elemsize == 0 )
    {
        return HB_FAILURE;
    }

    hb_vector_t * vector = HB_NEW( hb_vector_t );

    if( _count != 0 )
    {
        vector->data = HB_ALLOC( _elemsize * _count );
    }
    else
    {
        vector->data = HB_NULLPTR;
    }

    vector->elemsize = _elemsize;
    vector->count = _count;

    *_vector = vector;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_vector_destroy( hb_vector_t * _vector )
{
    if( _vector != HB_NULLPTR )
    {
        HB_FREE( _vector->data );
    }

    HB_DELETE( _vector );
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_vector_count( const hb_vector_t * _vector )
{
    return _vector->count;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vector_resize( hb_vector_t * _vector, size_t _count )
{
    if( _vector->count >= _count )
    {
        _vector->count = _count;

        return HB_SUCCESSFUL;
    }

    void * data = HB_REALLOC( _vector->data, _vector->elemsize * _count );

    if( data == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    _vector->data = data;
    _vector->count = _count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vector_set( hb_vector_t * _vector, uint32_t _index, const void * _element )
{
#ifdef HB_DEBUG
    if( _index >= _vector->count )
    {
        return HB_FAILURE;
    }
#endif

    memcpy( HB_MEMOFFSET( _vector->data, _vector->elemsize * _index ), _element, _vector->elemsize );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vector_get( const hb_vector_t * _vector, uint32_t _index, void ** _element )
{
#ifdef HB_DEBUG
    if( _index >= _vector->count )
    {
        return HB_FAILURE;
    }
#endif

    * _element = *HB_TMEMOFFSET( void **, _vector->data, _vector->elemsize * _index );

    return HB_SUCCESSFUL;
}
