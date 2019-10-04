#include "hb_getopt.h"

#include <string.h>

int hb_getopt( int argc, char * argv[], const char * _name, const char ** _value )
{
    for( int index = 1; index != argc; ++index )
    {
        if( strcmp( argv[index], _name ) != 0 )
        {
            continue;
        }

        if( index + 1 == argc )
        {
            return 0;
        }

        *_value = argv[index + 1];

        return 1;
    }

    return 0;
}