#include "hb_script.h"
#include "hb_script_json.h"
#include "hb_script_handle.h"

#include "hb_log/hb_log.h"
#include "hb_utils/hb_oid.h"

#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
extern int __hb_script_server_GetCurrentUserPublicData( lua_State * L );
extern int __hb_script_server_SetCurrentUserPublicData( lua_State * L );
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
    { "print", &__hb_lua_print }
    , { NULL, NULL } /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg serverLib[] = {
    { "GetCurrentUserPublicData", &__hb_script_server_GetCurrentUserPublicData }
    , { "SetCurrentUserPublicData", &__hb_script_server_SetCurrentUserPublicData }
    , { NULL, NULL } /* end of array */
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
hb_result_t hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_db_collection_handle_t * _ucollection, const hb_db_collection_handle_t * _pcollection, hb_oid_t _uuid, hb_oid_t _puid )
{
    g_script_handle = HB_NEW( hb_script_handle_t );
    
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
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
    lua_setglobal( L, "event" );

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

    g_script_handle->db_user_collection = _ucollection;
    g_script_handle->db_project_collection = _pcollection;

    hb_oid_copy( g_script_handle->user_oid, _uuid );
    hb_oid_copy( g_script_handle->project_oid, _puid );

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize()
{
    hb_log_message( "script", HB_LOG_INFO, "memory peak %d [max %d] %%%0.2f", g_script_handle->memory_peak - g_script_handle->memory_base, g_script_handle->memory_limit - g_script_handle->memory_base, (float)(g_script_handle->memory_peak - g_script_handle->memory_base) / (float)(g_script_handle->memory_limit - g_script_handle->memory_base) * 100.f );
    hb_log_message( "script", HB_LOG_INFO, "instruction %d [max %d] %%%0.2f", g_script_handle->call_used, g_script_handle->call_limit, (float)(g_script_handle->call_used) / (float)(g_script_handle->call_limit) * 100.f );

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
hb_result_t hb_script_load( const void * _buffer, size_t _size )
{
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    lua_State * L = g_script_handle->L;

    int status = luaL_loadbufferx( L, _buffer, _size, "script", HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * e = lua_tostring( L, -1 );
        hb_log_message( "script", HB_LOG_ERROR, "%s", e );

        lua_pop( L, 1 );  /* pop error message from the stack */

        return HB_FAILURE;
    }

    int ret = lua_pcallk( L, 0, 0, 0, 0, HB_NULLPTR );

    if( ret != 0 )
    {
        const char * e = lua_tostring( L, -1 );
        hb_log_message( "script", HB_LOG_ERROR, "%s", e );

        lua_pop( L, 1 );  /* pop error message from the stack */

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_server_call( const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_bool_t * _successful )
{
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    lua_State * L = g_script_handle->L;

    lua_getglobal( L, "api" );

    if( lua_getfield( L, -1, _method ) != LUA_TFUNCTION )
    {
        return HB_FAILURE;
    }

    if( hb_script_json_loads( L, _data, _datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, 2, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        hb_log_message( "script", HB_LOG_ERROR, "call function '%s' data '%.*s' with error: %s"
            , _method
            , _datasize
            , _data
            , error_msg
        );

        return HB_FAILURE;
    }

    int successful = lua_toboolean( L, -2 );
    
    if( lua_type( L, -1 ) == LUA_TTABLE )
    {
        hb_script_json_dumps( L, _result, _capacity, _resultsize );
    }
    else
    {
        sprintf( _result, "{}" );
    }

    lua_pop( L, 2 );

    *_successful = (hb_bool_t)successful;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_event_call( const char * _event, const void * _data, size_t _datasize )
{
    if( setjmp( g_script_handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    lua_State * L = g_script_handle->L;

    lua_getglobal( L, "event" );

    if( lua_getfield( L, -1, _event ) != LUA_TFUNCTION )
    {
        return HB_SUCCESSFUL;
    }

    if( hb_script_json_loads( L, _data, _datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, 0, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        hb_log_message( "script", HB_LOG_ERROR, "call function '%s' data '%.*s' with error: %s"
            , _event
            , _datasize
            , _data
            , error_msg
        );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////