#include "hb_script.h"

#include "hb_log/hb_log.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
lua_State * g_L = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
static const luaL_Reg serverReg[] = {
    {"GetCurrentUser", math_abs},
{"cos", math_cos},
{"sin", math_sin},
{NULL, NULL}
};
//////////////////////////////////////////////////////////////////////////
int hb_script_initialize()
{
    lua_State * L = luaL_newstate();

    luaopen_base( L );
    luaopen_coroutine( L );
    luaopen_table( L );
    luaopen_string( L );
    luaopen_utf8( L );
    luaopen_bit32( L );
    luaopen_math( L );

    g_L = L;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize()
{
    lua_close( g_L );

    g_L = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_load( const void * _buffer, size_t _size )
{
    int status = luaL_loadbufferx( g_L, _buffer, _size, "script", HB_NULLPTR );

    if( status != LUA_OK )
    {
        return 0;
    }

    lua_createtable( g_L, 0, 256 );
    lua_setglobal( g_L, "client" );

    lua_createtable( g_L, 0, 256 );
    lua_setglobal( g_L, "server" );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_call( const char * _method, const char * _data, char * _result, size_t _capacity )
{
    lua_getglobal( g_L, "client" );
    lua_getfield( g_L, -1, _method );
    lua_pushstring( g_L, _data );

    int status = lua_pcallk( g_L, 1, 1, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( g_L, -1, HB_NULLPTR );

        hb_log_message( HB_LOG_ERROR, "call function '%s' data '%s' with error: %s"
            , _method
            , _data
            , error_msg
        );

        return 0;
    }

    if( lua_isstring( g_L, -1 ) != LUA_OK )
    {
        hb_log_message( HB_LOG_ERROR, "call function '%s' must return a string"
            , _method
        );

        return 0;
    }

    size_t length;
    const char * value = lua_tolstring( g_L, -1, &length );

    if( length >= _capacity )
    {
        lua_pop( g_L, 1 );

        return 0;
    }

    strcpy( _result, value );

    lua_pop( g_L, 1 );

    return 1;
}