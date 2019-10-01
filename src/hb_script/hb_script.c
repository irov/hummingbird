#include "hb_script.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <string.h>
#include <setjmp.h>

//////////////////////////////////////////////////////////////////////////
lua_State * g_L = HB_NULLPTR;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_script_settings_t
{
    char user[32];

    hb_db_collection_handler_t db_collection;
} hb_script_settings_t;
//////////////////////////////////////////////////////////////////////////
hb_script_settings_t * g_settings;
//////////////////////////////////////////////////////////////////////////
static int __server_GetCurrentUserData( lua_State * L )
{
    const char * fields[16];

    uint32_t field_iterator = 0;

    lua_pushnil( L );
    while( lua_next( L, 1 ) != 0 )
    {
        const char * value = lua_tostring( L, -1 );
        fields[field_iterator++] = value;

        lua_pop( L, 1 );
    }

    hb_db_value_handler_t handler;
    hb_db_get_value( &g_settings->db_collection, g_settings->user, fields, field_iterator, &handler );

    for( uint32_t index = 0; index != field_iterator; ++index )
    {
        const char * value = handler.value[index];
        size_t length = handler.length[index];

        lua_pushlstring( L, value, length );
    }

    hb_db_value_destroy( &handler );

    return field_iterator;
}
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
static jmp_buf g_hb_lua_panic_jump;
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_panic( lua_State * L )
{
    HB_UNUSED( L );

    longjmp( g_hb_lua_panic_jump, 1 );
}
//////////////////////////////////////////////////////////////////////////
int hb_script_initialize(const char * _user )
{
    g_settings = HB_NEW( hb_script_settings_t );
    strcpy( g_settings->user, _user );

    if( hb_db_get_collection( "hb_users", "hb_data", &g_settings->db_collection ) == 0 )
    {
        return 0;
    }

    lua_State * L = luaL_newstate();

    if( setjmp( g_hb_lua_panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

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
    lua_pushstring( L, "GetCurrentUserData" );
    lua_pushcfunction( L, &__server_GetCurrentUserData );
    lua_settable( L, -3 );

    lua_setglobal( L, "server" );

    g_L = L;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize()
{
    hb_db_collection_destroy( &g_settings->db_collection );

    lua_close( g_L );

    g_L = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_load( const void * _buffer, size_t _size )
{
    int status = luaL_loadbufferx( g_L, _buffer, _size, "script", HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * e = lua_tostring( g_L, -1 );
        hb_log_message( HB_LOG_ERROR, "%s", e );

        lua_pop( g_L, 1 );  /* pop error message from the stack */

        return 0;
    }

    int ret = lua_pcallk( g_L, 0, 0, 0, 0, HB_NULLPTR );

    if( ret != 0 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_call( const char * _method, const char * _data, size_t _size, char * _result, size_t _capacity )
{
    HB_UNUSED( _capacity );

    lua_getglobal( g_L, "server" );
    lua_getfield( g_L, -1, _method );
    
    int res = luaL_loadbufferx( g_L, _data, _size, HB_NULLPTR, HB_NULLPTR );

    if( res != LUA_OK )
    {
        const char * e = lua_tostring( g_L, -1 );
        hb_log_message( HB_LOG_ERROR, "%s", e );

        return 0;
    }

    int status2 = lua_pcallk( g_L, 0, 1, 0, 0, HB_NULLPTR );

    if( status2 != LUA_OK )
    {
        const char * error_msg = lua_tolstring( g_L, -1, HB_NULLPTR );

        hb_log_message( HB_LOG_ERROR, "call function '%s' data '%s' with error: %s"
            , _method
            , _data
            , error_msg
        );

        return 0;
    }
    
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

    strcpy( _result, "{" );

    lua_pushnil( g_L );
    int it = lua_next( g_L, -2 );
    while( it != 0 )
    {
        const char * key = lua_tostring( g_L, -2 );

        if( lua_isinteger( g_L, -1 ) == 1 )
        {
            const char * value = lua_tostring( g_L, -1 );

            strcat( _result, key );
            strcat( _result, "=" );
            strcat( _result, value );
        }
        else if( lua_isnumber( g_L, -1 ) == 1 )
        {
            const char * value = lua_tostring( g_L, -1 );

            strcat( _result, key );
            strcat( _result, "=" );
            strcat( _result, value );
        }
        else if( lua_isstring( g_L, -1 ) == 1 )
        {
            const char * value = lua_tostring( g_L, -1 );

            strcat( _result, key );
            strcat( _result, "=\"" );
            strcat( _result, value );
            strcat( _result, "\"" );
        }

        lua_pop( g_L, 1 );

        it = lua_next( g_L, -2 );

        if( it == 0 )
        {
            break;
        }

        strcat( _result, "," );
    }

    strcat( _result, "}" );

    lua_pop( g_L, 1 );

    return 1;
}