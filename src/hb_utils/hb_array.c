#include "hb_array.h"

#include "hb_memory/hb_memory.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct hb_array_t
{
    void * data;
    hb_size_t size;
} hb_array_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_array_create( const void * _data, hb_size_t _size, hb_array_t ** _array )
{
    hb_array_t * array = HB_NEW( hb_array_t );

    array->data = HB_ALLOC( _size );
    array->size = _size;

    memcpy( array->data, _data, _size );

    *_array = array;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_array_destroy( hb_array_t * _array )
{
    HB_FREE( _array->data );

    HB_DELETE( _array );
}
//////////////////////////////////////////////////////////////////////////
const void * hb_array_data( hb_array_t * _array, hb_size_t * _size )
{
    *_size = _array->size;

    return _array->data;
}
//////////////////////////////////////////////////////////////////////////