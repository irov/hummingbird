#include "hb_getopt.h"

#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_getopt( int argc, char * argv[], const char * _name, const char ** _value )
{
    for( int index = 1; index != argc; ++index )
    {
        if( strcmp( argv[index], _name ) != 0 )
        {
            continue;
        }

        if( index + 1 == argc )
        {
            return HB_FAILURE;
        }

        *_value = argv[index + 1];

        return HB_SUCCESSFUL;
    }

    return HB_FAILURE;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_getopti( int argc, char * argv[], const char * _name, int64_t * const _value )
{
    const char * value;
    if( hb_getopt( argc, argv, _name, &value ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    char * endptr;
    int64_t v = strtoll( value, &endptr, 0 );

    if( *endptr != '\0' )
    {
        return HB_FAILURE;
    }

    *_value = v;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////