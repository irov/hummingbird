#include "hb_script_handle.h"

#include <stdarg.h>

void hb_script_error( lua_State * L, const char * _format, ... )
{
    va_list vargs;
    va_start( vargs, _format );

    char msg[4096] = {0};
    vsprintf( msg, _format, vargs );
    lua_pushstring( L, msg );
    va_end( vargs );

    lua_error( L );
}