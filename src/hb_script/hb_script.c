#include "hb_script.h"

#include "hb_log/hb_log.h"

#include "hb_script_handle.h"

#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_script_handle_t * g_script_handle;
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

    longjmp( g_script_handle->panic_jump, 1 );
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_lua_alloc( void * ud, void * ptr, size_t osize, size_t nsize )
{
    HB_UNUSED( ud );

    hb_script_handle_t * g = g_script_handle;

    if( ptr == HB_NULLPTR )
    {
        osize = 0;
    }

    if( g->memory_limit < g->memory_used + nsize - osize )
    {
        return HB_NULLPTR;
    }

    g->memory_used += nsize;
    g->memory_used -= osize;

    if( g->memory_used > g->memory_peak )
    {
        g->memory_peak = g->memory_used;
    }
    
    if( nsize == 0 )
    {
        free( ptr );

        return HB_NULLPTR;
    }
    else
    {
        void * nptr = realloc( ptr, nsize );

        return nptr;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __hb_lua_hook( lua_State * L, lua_Debug * ar )
{
    HB_UNUSED( L );
    HB_UNUSED( ar );

    if( ++g_script_handle->call_used == g_script_handle->call_limit )
    {
        luaL_error( L, "call limit" );
    }

    return;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_initialize( size_t _memorylimit, size_t _calllimit )
{
    g_script_handle = HB_NEW( hb_script_handle_t );
    
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    g_script_handle->memory_base = 0;
    g_script_handle->memory_used = 0;
    g_script_handle->memory_peak = 0;
    g_script_handle->memory_limit = _memorylimit;

    lua_State * L = lua_newstate( &__hb_lua_alloc, HB_NULLPTR );

    lua_gc( L, LUA_GCCOLLECT, 0 );
    lua_gc( L, LUA_GCSTOP, 0 );

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
    lua_setglobal( L, "server" );

    lua_getglobal( L, "server" );
    luaL_setfuncs( L, serverLib, 0 );

    g_script_handle->call_used = 0;
    g_script_handle->call_limit = _calllimit;
    lua_sethook( L, &__hb_lua_hook, LUA_MASKCOUNT, 1 );

    size_t memory_used = g_script_handle->memory_used;
    g_script_handle->memory_base = memory_used;
    g_script_handle->memory_limit += memory_used;

    g_script_handle->L = L;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize()
{
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return;
    }

    lua_close( g_script_handle->L );
    g_script_handle->L = HB_NULLPTR;

    HB_DELETE( g_script_handle );
    g_script_handle = HB_NULLPTR;
}
//////////////////////////////////////////////////////////////////////////
int hb_script_user_initialize( const char * _user )
{
    strcpy( g_script_handle->user, _user );

    if( hb_db_get_collection( "hb_users", "hb_data", &g_script_handle->db_collection ) == 0 )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_user_finalize()
{
    hb_log_message( "script", HB_LOG_INFO, "memory peak %d [max %d] %%%0.2f", g_script_handle->memory_peak - g_script_handle->memory_base, g_script_handle->memory_limit - g_script_handle->memory_base, (float)(g_script_handle->memory_peak - g_script_handle->memory_base) / (float)(g_script_handle->memory_limit - g_script_handle->memory_base) * 100.f );
    hb_log_message( "script", HB_LOG_INFO, "instruction %d [max %d] %%%0.2f", g_script_handle->call_used, g_script_handle->call_limit, (float)(g_script_handle->call_used) / (float)(g_script_handle->call_limit) * 100.f );

    hb_db_collection_destroy( &g_script_handle->db_collection );
}
//////////////////////////////////////////////////////////////////////////
int hb_script_user_load( const void * _buffer, size_t _size )
{
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    lua_State * L = g_script_handle->L;

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
int hb_script_user_call( const char * _method, size_t _methodsize, const char * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize )
{
    HB_UNUSED( _capacity );

    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return 0;
    }

    lua_State * L = g_script_handle->L;

    lua_getglobal( L, "api" );

    char lua_method[64];
    strncpy( lua_method, _method, _methodsize );
    lua_method[_methodsize] = '\0';

    lua_getfield( L, -1, lua_method );
    
    char lua_data[2048] = {"return "};
    strncat( lua_data, _data, _datasize );

    int res = luaL_loadbufferx( L, lua_data, _datasize + sizeof( "return " ) - 1, HB_NULLPTR, HB_NULLPTR );

    if( res != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        hb_log_message( "script", HB_LOG_ERROR, "%s"
            , error_msg 
        );

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

    int successful = lua_toboolean( L, -2 );

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

    *_resultsize = strlen( _result );

    lua_pop( L, 2 );

    if( successful == 0 )
    {
        return -1;
    }

    return 1;
}