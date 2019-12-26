#include "hb_httpopt.h"

#include <string.h>

hb_result_t hb_httpopt( const char * _http, size_t _size, const char * _name, const char ** _value, size_t * _valuesize )
{
    const char * base = _http;

    while( base != HB_NULLPTR )
    {
        const char * chr_equal = strchr( base, '=' );
        const char * chr_amp = strchr( base, '&' );

        const char * chr_equal_name = base;
        size_t chr_equal_name_size = chr_equal - base;

        if( chr_amp == HB_NULLPTR )
        {
            base = HB_NULLPTR;
        }
        else
        {
            base = chr_amp + 1;
        }

        if( strncmp( chr_equal_name, _name, chr_equal_name_size ) != 0 )
        {
            continue;
        }

        *_value = chr_equal + 1;

        if( chr_amp == HB_NULLPTR )
        {
            *_valuesize = _size - (chr_equal - _http) - 1;
        }
        else
        {
            *_valuesize = chr_amp - chr_equal - 1;
        }

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}