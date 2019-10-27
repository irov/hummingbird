#include "hb_script_json.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_script_json_value_dumps( lua_State * L, char * _buffer, size_t * _offset, size_t _capacity )
{
    HB_UNUSED( _capacity );

    int type = lua_type( L, -1 );

    switch( type )
    {
    case LUA_TBOOLEAN:
        {
            int result = lua_toboolean( L, -1 );

            *_offset += sprintf( _buffer + *_offset, result == 1 ? "true" : "false" );
        }break;
    case LUA_TNUMBER:
        {
            lua_Number result = lua_tonumber( L, -1 );

            *_offset += sprintf( _buffer + *_offset, LUA_NUMBER_FMT, result );
        }break;
    case LUA_TSTRING:
        {
            const char * result = lua_tostring( L, -1 );

            *_offset += sprintf( _buffer + *_offset, "%s", result );
        }break;
    case LUA_TTABLE:
        {
            *_offset += sprintf( _buffer + *_offset, "{" );

            lua_pushnil( L );
            while( lua_next( L, -2 ) != 0 )
            {
                const char * key = lua_tostring( L, -2 );
                *_offset += sprintf( _buffer + *_offset, "\"%s\": ", key );

                if( __hb_script_json_value_dumps( L, _buffer, _offset, _capacity ) == HB_FAILURE )
                {
                    return HB_FAILURE;
                }

                lua_pop( L, 1 );
            }

            *_offset += sprintf( _buffer + *_offset, "}" );
        }break;
    default:
        return HB_FAILURE;
    }

    lua_pop( L, 1 );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_json_dumps( lua_State * L, char * _buffer, size_t _capacity, size_t * _size )
{
    size_t offset = 0;

    if( __hb_script_json_value_dumps( L, _buffer, &offset, _capacity ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    *_size = offset;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////