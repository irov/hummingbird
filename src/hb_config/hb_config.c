#include "hb_config.h"

#include <malloc.h>

#ifdef HB_DEBUG
#include <string.h>
#endif

//////////////////////////////////////////////////////////////////////////
void * hb_new( size_t _size )
{
    void * p = malloc( _size );

#ifdef HB_DEBUG
    if( p == HB_NULLPTR )
    {
        return HB_NULLPTR;
    }

    memset( p, 0xcd, _size );
#endif

    return p;
}
//////////////////////////////////////////////////////////////////////////
void hb_free( void * _ptr )
{
    free( _ptr );
}
//////////////////////////////////////////////////////////////////////////