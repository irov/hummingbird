#include "hb_getenv.h"

#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_getenv( const char * _name, char * const _value, size_t _capacity )
{
    char * value = getenv( _name );

    if( value == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    strncpy( _value, value, _capacity );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_getenvi( const char * _name, int64_t * const _value )
{
    char * value = getenv( _name );

    if( value == HB_NULLPTR )
    {
        return HB_FAILURE;
    }

    char * endptr;
    int64_t v = strtoll( value, &endptr, 10 );

    if( *endptr != '\0' )
    {
        return HB_FAILURE;
    }

    *_value = v;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////