#include "hb_strncpyn.h"

#include <memory.h>

//////////////////////////////////////////////////////////////////////////
hb_bool_t hb_strncpyn( char * _dst, hb_size_t _capacity, const char * _src, hb_size_t _size )
{
    if( _size >= _capacity )
    {
        return HB_FALSE;
    }

    memcpy( _dst, _src, _size );
    _dst[_size] = '\0';

    return HB_TRUE;
}
//////////////////////////////////////////////////////////////////////////