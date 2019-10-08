#include "hb_strstre.h"

#include <string.h>

const char * hb_strstre( const char * _str, const char * _substr )
{
    const char * r = strstr( _str, _substr );

    if( r == HB_NULLPTR )
    {
        return r;
    }

    r += strlen( _substr );

    return r;
}