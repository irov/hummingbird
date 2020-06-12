#include "hb_vectorptr.h"

#include "hb_memory/hb_memory.h"

//////////////////////////////////////////////////////////////////////////
static const size_t HB_VECTORPTR_ELEMENT_SIZE = sizeof( void * );
//////////////////////////////////////////////////////////////////////////
typedef struct hb_vectorptr_t
{
    void * data;
    uint32_t count;
} hb_vectorptr_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vectorptr_create( uint32_t _count, hb_vectorptr_t ** _vector )
{
    hb_vectorptr_t * vector = HB_NEW( hb_vectorptr_t );

    if( _count != 0 )
    {
        vector->data = HB_ALLOC( HB_VECTORPTR_ELEMENT_SIZE * _count );
    }
    else
    {
        vector->data = HB_NULLPTR;
    }

    vector->count = _count;

    *_vector = vector;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_vectorptr_destroy( hb_vectorptr_t * _vector )
{
    if( _vector != HB_NULLPTR )
    {
        HB_FREE( _vector->data );
    }

    HB_DELETE( _vector );
}
//////////////////////////////////////////////////////////////////////////
uint32_t hb_vectorptr_count( const hb_vectorptr_t * _vector )
{
    return _vector->count;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vectorptr_resize( hb_vectorptr_t * _vector, size_t _count )
{
    if( _vector->count >= _count )
    {
        _vector->count = _count;

        return HB_SUCCESSFUL;
    }

    void * data = HB_REALLOC( _vector->data, HB_VECTORPTR_ELEMENT_SIZE * _count );

    if( data == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    _vector->data = data;
    _vector->count = _count;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vectorptr_set( hb_vectorptr_t * _vector, uint32_t _index, void * _ptr )
{
#ifdef HB_DEBUG
    if( _index >= _vector->count )
    {
        return HB_FAILURE;
    }
#endif

    * HB_TMEMOFFSET( void **, _vector->data, HB_VECTORPTR_ELEMENT_SIZE * _index ) = _ptr;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_vectorptr_get( const hb_vectorptr_t * _vector, uint32_t _index, void ** _ptr )
{
#ifdef HB_DEBUG
    if( _index >= _vector->count )
    {
        return HB_FAILURE;
    }
#endif

    * _ptr = *HB_TMEMOFFSET( void **, _vector->data, HB_VECTORPTR_ELEMENT_SIZE * _index );

    return HB_SUCCESSFUL;
}
