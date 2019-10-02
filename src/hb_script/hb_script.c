#include "hb_script.h"

#include "hb_log/hb_log.h"

#include "hb_script_settings.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_script_settings_t * g_script_settings;
//////////////////////////////////////////////////////////////////////////
extern int __hb_script_server_GetCurrentUserData( lua_State * L );
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_print( lua_State * L )
{
    int n = lua_gettop( L );  /* number of arguments */
    int i;
    lua_getglobal( L, "tostring" );
    for( i = 1; i <= n; i++ )
    {
        const char * s;
        size_t l;
        lua_pushvalue( L, -1 );  /* function to be called */
        lua_pushvalue( L, i );   /* value to print */
        lua_call( L, 1, 1 );
        s = lua_tolstring( L, -1, &l );  /* get result */
        if( s == NULL )
            return luaL_error( L, "'tostring' must return a string to 'print'" );
        if( i > 1 ) printf( "\t" );
        printf( s );
        lua_pop( L, 1 );  /* pop result */
    }
    printf("\n");

    return 0;
}
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg globalLib[] = {
    {"print", &__hb_lua_print},
{NULL, NULL} /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg serverLib[] = {
    { "GetCurrentUserData", &__hb_script_server_GetCurrentUserData },
{ NULL, NULL } /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_panic( lua_State * L )
{
    HB_UNUSED( L );

    longjmp( g_script_settings->panic_jump, 1 );
}
//////////////////////////////////////////////////////////////////////////
int hb_script_initialize(const char * _user )
{
    g_script_settings = HB_NEW( hb_script_settings_t );
    strcpy( g_script_settings->user, _user );

    if( hb_db_get_collection( "hb_users", "hb_data", &g_script_settings->db_collection ) == 0 )
    {
        return 0;
    }

    if( setjmp( g_script_settings->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    lua_State * L = luaL_newstate();

    lua_atpanic( L, &__hb_lua_panic );

    luaopen_base( L );
    luaopen_coroutine( L );
    luaopen_table( L );
    luaopen_string( L );
    luaopen_utf8( L );
    luaopen_math( L );

    lua_getglobal( L, "_G" );
    luaL_setfuncs( L, globalLib, 0 );

    lua_newtable( L );
    lua_setglobal( L, "api" );
    
    lua_newtable( L );
    luaL_setfuncs( L, serverLib, 0 );
    lua_setglobal( L, "server" );

    g_script_settings->L = L;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize()
{
    if( setjmp( g_script_settings->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return;
    }

    hb_db_collection_destroy( &g_script_settings->db_collection );

    lua_close( g_script_settings->L );
    g_script_settings->L = HB_NULLPTR;

    HB_DELETE( g_script_settings );
    g_script_settings = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_load( const void * _buffer, size_t _size )
{
    if( setjmp( g_script_settings->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    lua_State * L = g_script_settings->L;

    int status = luaL_loadbufferx( L, _buffer, _size, "script", HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * e = lua_tostring( L, -1 );
        hb_log_message( "script", HB_LOG_ERROR, "%s", e );

        lua_pop( L, 1 );  /* pop error message from the stack */

        return 0;
    }

    int ret = lua_pcallk( L, 0, 0, 0, 0, HB_NULLPTR );

    if( ret != 0 )
    {
        const char * e = lua_tostring( L, -1 );
        hb_log_message( "script", HB_LOG_ERROR, "%s", e );

        lua_pop( L, 1 );  /* pop error message from the stack */

        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_call( const char * _method, const char * _data, size_t _size, char * _result, size_t _capacity )
{
    HB_UNUSED( _capacity );

    if( setjmp( g_script_settings->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    lua_State * L = g_script_settings->L;

    lua_getglobal( L, "api" );
    lua_getfield( L, -1, _method );
    
    int res = luaL_loadbufferx( L, _data, _size, HB_NULLPTR, HB_NULLPTR );

    if( res != LUA_OK )
    {
        const char * e = lua_tostring( L, -1 );
        hb_log_message( "script", HB_LOG_ERROR, "%s", e );

        return 0;
    }

    int status2 = lua_pcallk( L, 0, 1, 0, 0, HB_NULLPTR );

    if( status2 != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        hb_log_message( "script", HB_LOG_ERROR, "call function '%s' data '%s' with error: %s"
            , _method
            , _data
            , error_msg
        );

        return 0;
    }
    
    int status = lua_pcallk( L, 1, 2, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        hb_log_message( "script", HB_LOG_ERROR, "call function '%s' data '%s' with error: %s"
            , _method
            , _data
            , error_msg
        );

        return 0;
    }

    lua_Integer successful = lua_tointeger( L, -2 );

    if( successful == 0 )
    {
        strcpy( _result, "{}" );

        return 1;
    }

    strcpy( _result, "{" );

    lua_pushnil( L );
    int it = lua_next( L, -2 );
    while( it != 0 )
    {
        const char * key = lua_tostring( L, -2 );

        if( lua_isinteger( L, -1 ) == 1 )
        {
            const char * value = lua_tostring( L, -1 );

            strcat( _result, key );
            strcat( _result, "=" );
            strcat( _result, value );
        }
        else if( lua_isnumber( L, -1 ) == 1 )
        {
            const char * value = lua_tostring( L, -1 );

            strcat( _result, key );
            strcat( _result, "=" );
            strcat( _result, value );
        }
        else if( lua_isstring( L, -1 ) == 1 )
        {
            const char * value = lua_tostring( L, -1 );

            strcat( _result, key );
            strcat( _result, "=\"" );
            strcat( _result, value );
            strcat( _result, "\"" );
        }

        lua_pop( L, 1 );

        it = lua_next( L, -2 );

        if( it == 0 )
        {
            break;
        }

        strcat( _result, "," );
    }

    strcat( _result, "}" );

    lua_pop( L, 1 );

    return 1;
}