#include "hb_script.h"
#include "hb_script_json.h"
#include "hb_script_handle.h"

#include "hb_log/hb_log.h"
#include "hb_storage/hb_storage.h"
#include "hb_utils/hb_oid.h"

#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
extern int __hb_script_server_GetProjectPublicData( lua_State * L );
extern int __hb_script_server_SetProjectPublicData( lua_State * L );
extern int __hb_script_server_UpdateProjectPublicData( lua_State * L );
extern int __hb_script_server_GetCurrentUserPublicData( lua_State * L );
extern int __hb_script_server_SetCurrentUserPublicData( lua_State * L );
extern int __hb_script_server_UpdateCurrentUserPublicData( lua_State * L );
extern int __hb_script_server_GetUserEntityPublicData( lua_State * L );
extern int __hb_script_server_SetUserEntityPublicData( lua_State * L );
extern int __hb_script_server_UpdateUserEntityPublicData( lua_State * L );
extern int __hb_script_server_CreateUserEntity( lua_State * L );
extern int __hb_script_server_SelectUserEntity( lua_State * L );
extern int __hb_script_server_GetUserEntityPublicData( lua_State * L );
extern int __hb_script_server_SetUserEntityPublicData( lua_State * L );
extern int __hb_script_server_UpdateUserEntityPublicData( lua_State * L );
extern int __hb_script_server_CreateProjectEntity( lua_State * L );
extern int __hb_script_server_GetProjectEntity( lua_State * L );
extern int __hb_script_server_SelectProjectEntity( lua_State * L );
extern int __hb_script_server_GetProjectEntityPublicData( lua_State * L );
extern int __hb_script_server_SetProjectEntityPublicData( lua_State * L );
extern int __hb_script_server_CreateMatching( lua_State * L );
extern int __hb_script_server_JoinMatching( lua_State * L );
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_print( lua_State * L )
{
    char msg[1024] = { '\0' };

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
        if( i > 1 ) strcat( msg, "\t" );
        strcat( msg, s );
        lua_pop( L, 1 );  /* pop result */
    }
    strcat( msg, "\n" );

    HB_LOG_MESSAGE_INFO( "script", "%s", msg );

    return 0;
}
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_json_dumps( lua_State * L )
{
    char buffer[10240];
    size_t buffer_size;
    if( hb_script_json_dumps( L, -1, buffer, 10240, &buffer_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushlstring( L, buffer, buffer_size );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg globalLib[] = {
    { "print", &__hb_lua_print }
    , { "json_dumps", &__hb_lua_json_dumps }
    , { NULL, NULL } /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg serverLib[] = {
    { "GetProjectPublicData", &__hb_script_server_GetProjectPublicData }
    , { "SetProjectPublicData", &__hb_script_server_SetProjectPublicData }
    , { "UpdateProjectPublicData", &__hb_script_server_UpdateProjectPublicData }
    , { "GetCurrentUserPublicData", &__hb_script_server_GetCurrentUserPublicData }
    , { "SetCurrentUserPublicData", &__hb_script_server_SetCurrentUserPublicData }
    , { "UpdateCurrentUserPublicData", &__hb_script_server_UpdateCurrentUserPublicData }
    , { "GetUserEntityPublicData", &__hb_script_server_GetUserEntityPublicData }
    , { "SetUserEntityPublicData", &__hb_script_server_SetUserEntityPublicData }
    , { "UpdateCurrentUserPublicData", &__hb_script_server_UpdateCurrentUserPublicData }
    , { "CreateUserEntity", &__hb_script_server_CreateUserEntity }
    , { "SelectUserEntity", &__hb_script_server_SelectUserEntity }
    , { "SetUserEntityPublicData", &__hb_script_server_SetUserEntityPublicData }
    , { "GetUserEntityPublicData", &__hb_script_server_GetUserEntityPublicData }
    , { "UpdateUserEntityPublicData", &__hb_script_server_UpdateUserEntityPublicData }
    , { "CreateProjectEntity", &__hb_script_server_CreateProjectEntity }
    , { "SelectProjectEntity", &__hb_script_server_SelectProjectEntity }
    , { "GetProjectEntity", &__hb_script_server_GetProjectEntity }
    , { "SetUserEntityPublicData", &__hb_script_server_SetUserEntityPublicData }
    , { "GetUserEntityPublicData", &__hb_script_server_GetUserEntityPublicData }
    , { "UpdateUserEntityPublicData", &__hb_script_server_UpdateUserEntityPublicData }
    , { "CreateMatching", &__hb_script_server_CreateMatching }
    , { "JoinMatching", &__hb_script_server_JoinMatching }
    , { NULL, NULL } /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_panic( lua_State * L )
{
    hb_script_handle_t * handle = *(hb_script_handle_t **)lua_getextraspace( L );

    longjmp( handle->panic_jump, 1 );
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_lua_alloc( void * ud, void * ptr, size_t osize, size_t nsize )
{
    hb_script_handle_t * handle = (hb_script_handle_t *)ud;

    if( ptr == HB_NULLPTR )
    {
        osize = 0;
    }

    if( handle->memory_limit < handle->memory_used + nsize - osize )
    {
        return HB_NULLPTR;
    }

    handle->memory_used += nsize;
    handle->memory_used -= osize;

    if( handle->memory_used > handle->memory_peak )
    {
        handle->memory_peak = handle->memory_used;
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
    HB_UNUSED( ar );

    hb_script_handle_t * handle = *(hb_script_handle_t **)lua_getextraspace( L );

    if( ++handle->call_used == handle->call_limit )
    {
        luaL_error( L, "call limit" );
    }
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_initialize( size_t _memorylimit, size_t _calllimit, const hb_oid_t _poid, const hb_oid_t _uoid, hb_matching_t * _matching, hb_script_handle_t ** _handle )
{
    hb_script_handle_t * handle = HB_NEW( hb_script_handle_t );

    if( hb_db_get_collection( "hb", "hb_user_entities", &handle->db_collection_user_entities ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_user_entities"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_project_entities", &handle->db_collection_project_entities ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_project_entities"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_users", &handle->db_collection_users ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_users"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_projects", &handle->db_collection_projects ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_matching", &handle->db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_matching"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_worlds", &handle->db_collection_worlds ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_worlds"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( "hb", "hb_avatars", &handle->db_collection_avatars ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "hb_avatars"
        );

        return HB_FAILURE;
    }


    hb_oid_copy( handle->project_oid, _poid );
    hb_oid_copy( handle->user_oid, _uoid );
    handle->matching = _matching;

    if( setjmp( handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    handle->memory_base = 0;
    handle->memory_used = 0;
    handle->memory_peak = 0;
    handle->memory_limit = ~0U;

    lua_State * L = lua_newstate( &__hb_lua_alloc, handle );

    lua_gc( L, LUA_GCCOLLECT, 0 );
    lua_gc( L, LUA_GCSTOP, 0 );

    lua_atpanic( L, &__hb_lua_panic );

    luaL_openlibs( L );
    //luaopen_package( L );
    //luaopen_coroutine( L );
    //luaopen_table( L );
    //luaopen_string( L );
    //luaopen_utf8( L );
    //luaopen_math( L );

    lua_getglobal( L, "_G" );
    luaL_setfuncs( L, globalLib, 0 );

    lua_newtable( L );
    lua_setglobal( L, "api" );

    lua_newtable( L );
    lua_setglobal( L, "event" );

    lua_newtable( L );
    lua_setglobal( L, "command" );

    lua_newtable( L );
    lua_setglobal( L, "server" );

    lua_getglobal( L, "server" );
    luaL_setfuncs( L, serverLib, 0 );

    handle->call_used = 0;
    handle->call_limit = _calllimit;
    lua_sethook( L, &__hb_lua_hook, LUA_MASKCOUNT, 1 );

    size_t memory_used = handle->memory_used;
    handle->memory_base = memory_used;
    handle->memory_limit = memory_used + _memorylimit;

    handle->L = L;

    *(hb_script_handle_t **)lua_getextraspace( L ) = handle;

    const char * db_projects_fields[] = { "script_sha1" };

    hb_db_value_handle_t project_values[1];
    if( hb_db_get_values( handle->db_collection_projects, _poid, db_projects_fields, project_values, 1 ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' not found 'script_sha1'"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    if( project_values[0].u.binary.length != sizeof( hb_sha1_t ) )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid data 'script_sha1'"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    hb_sha1_t script_sha1;
    memcpy( script_sha1, project_values[0].u.binary.buffer, sizeof( hb_sha1_t ) );

    hb_db_destroy_values( project_values, 1 );

    size_t script_data_size;
    hb_data_t script_data;
    if( hb_storage_get_code( script_sha1, script_data, sizeof( script_data ), &script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid get data from storage"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    if( hb_script_load( handle, script_data, script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid load data"
            , "hb_projects"
        );

        return HB_FAILURE;
    }

    *_handle = handle;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void hb_script_finalize( hb_script_handle_t * _handle )
{
    hb_db_destroy_collection( _handle->db_collection_users );
    hb_db_destroy_collection( _handle->db_collection_projects );

    HB_LOG_MESSAGE_INFO( "script", "memory peak %d [max %d] %%%0.2f", _handle->memory_peak - _handle->memory_base, _handle->memory_limit - _handle->memory_base, (float)(_handle->memory_peak - _handle->memory_base) / (float)(_handle->memory_limit - _handle->memory_base) * 100.f );
    HB_LOG_MESSAGE_INFO( "script", "instruction %d [max %d] %%%0.2f", _handle->call_used, _handle->call_limit, (float)(_handle->call_used) / (float)(_handle->call_limit) * 100.f );

    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return;
    }

    lua_close( _handle->L );
    _handle->L = HB_NULLPTR;

    HB_DELETE( _handle );
}
//////////////////////////////////////////////////////////////////////////
void hb_script_stat( hb_script_handle_t * _handle, hb_script_stat_t * _stat )
{
    _stat->memory_used = _handle->memory_peak - _handle->memory_base;
    _stat->call_used = _handle->call_used;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_load( hb_script_handle_t * _handle, const void * _buffer, size_t _size )
{
    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    lua_State * L = _handle->L;

    int status = luaL_loadbufferx( L, _buffer, _size, "script", HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * e = lua_tostring( L, -1 );
        HB_LOG_MESSAGE_ERROR( "script", "invalid load buffer: %s"
            , e
        );

        lua_pop( L, 1 );

        return HB_FAILURE;
    }

    int ret = lua_pcallk( L, 0, 0, 0, 0, HB_NULLPTR );

    if( ret != 0 )
    {
        const char * e = lua_tostring( L, -1 );

        HB_LOG_MESSAGE_ERROR( "script", "invalid call buffer: %s"
            , e
        );

        lua_pop( L, 1 );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_api_call( hb_script_handle_t * _handle, const char * _method, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code )
{
    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "script", "call api '%s' data '%.*s'", _method, _datasize, (const char *)_data );

    lua_State * L = _handle->L;

    int base = lua_gettop( L );

    lua_getglobal( L, "api" );

    if( lua_getfield( L, -1, _method ) != LUA_TFUNCTION )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    lua_remove( L, -2 );

    if( hb_script_json_loads( L, _data, _datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, LUA_MULTRET, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        HB_LOG_MESSAGE_ERROR( "script", "call function '%s' data '%.*s' with error: %s"
            , _method
            , _datasize
            , _data
            , error_msg
        );

        *_code = HB_ERROR_INTERNAL;

        return HB_SUCCESSFUL;
    }

    int top = lua_gettop( L );

    int nresults = top - base;

    if( nresults == 1 && lua_type( L, -1 ) == LUA_TTABLE )
    {
        if( hb_script_json_dumps( L, -1, _result, _capacity, _resultsize ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }

        lua_pop( L, 1 );
    }
    else if( nresults == 0 )
    {
        sprintf( _result, "{}" );

        *_resultsize = 2;
    }
    else
    {
        lua_pop( L, nresults );

        return HB_FAILURE;
    }

    *_code = HB_ERROR_OK;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_event_call( hb_script_handle_t * _handle, const char * _event, const void * _data, size_t _datasize )
{
    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "script", "call event '%s' data '%.*s'", _event, _datasize, (const char *)_data );

    lua_State * L = _handle->L;

    lua_getglobal( L, "event" );

    if( lua_getfield( L, -1, _event ) != LUA_TFUNCTION )
    {
        return HB_SUCCESSFUL;
    }

    lua_remove( L, -2 );

    if( hb_script_json_loads( L, _data, _datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, 0, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        HB_LOG_MESSAGE_ERROR( "script", "call function '%s' data '%.*s' with error: %s"
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
hb_result_t hb_script_command_call( hb_script_handle_t * _handle, const char * _command, const void * _data, size_t _datasize, char * _result, size_t _capacity, size_t * _resultsize, hb_error_code_t * _code )
{
    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "script", "call command '%s' data '%.*s'", _command, _datasize, (const char *)_data );

    lua_State * L = _handle->L;

    lua_getglobal( L, "command" );

    if( lua_getfield( L, -1, _command ) != LUA_TFUNCTION )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    lua_remove( L, -2 );

    if( hb_script_json_loads( L, _data, _datasize ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, 1, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        HB_LOG_MESSAGE_ERROR( "script", "call function '%s' data '%.*s' with error: %s"
            , _command
            , _datasize
            , _data
            , error_msg
        );

        *_code = HB_ERROR_INTERNAL;

        return HB_SUCCESSFUL;
    }

    if( lua_type( L, -1 ) == LUA_TTABLE )
    {
        if( hb_script_json_dumps( L, -1, _result, _capacity, _resultsize ) == HB_FAILURE )
        {
            return HB_FAILURE;
        }
    }
    else
    {
        sprintf( _result, "{}" );

        *_resultsize = 2;
    }

    lua_pop( L, 1 );

    *_code = HB_ERROR_OK;

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////