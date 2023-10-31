#include "hb_script.h"
#include "hb_script_json.h"
#include "hb_script_handle.h"

#include "hb_memory/hb_memory.h"
#include "hb_log/hb_log.h"
#include "hb_storage/hb_storage.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
extern int hb_script_server_GetProjectPublicData( lua_State * L );
extern int hb_script_server_SetProjectPublicData( lua_State * L );
extern int hb_script_server_UpdateProjectPublicData( lua_State * L );
extern int hb_script_server_GetCurrentUserPublicData( lua_State * L );
extern int hb_script_server_SetCurrentUserPublicData( lua_State * L );
extern int hb_script_server_UpdateCurrentUserPublicData( lua_State * L );
extern int hb_script_server_GetUserEntityPublicData( lua_State * L );
extern int hb_script_server_SetUserEntityPublicData( lua_State * L );
extern int hb_script_server_UpdateUserEntityPublicData( lua_State * L );
extern int hb_script_server_CreateUserEntity( lua_State * L );
extern int hb_script_server_SelectUserEntity( lua_State * L );
extern int hb_script_server_CreateProjectEntity( lua_State * L );
extern int hb_script_server_GetProjectEntity( lua_State * L );
extern int hb_script_server_SelectProjectEntity( lua_State * L );
extern int hb_script_server_GetProjectEntityPublicData( lua_State * L );
extern int hb_script_server_SetProjectEntityPublicData( lua_State * L );
extern int hb_script_server_CreateMatching( lua_State * L );
extern int hb_script_server_JoinMatching( lua_State * L );
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_print( lua_State * L )
{
    char msg[1024] = {'\0'};

    int n = lua_gettop( L );  /* number of arguments */
    int i;
    lua_getglobal( L, "tostring" );
    for( i = 1; i <= n; i++ )
    {
        const char * s;
        hb_size_t l;
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
    char buffer[HB_DATA_MAX_SIZE];
    hb_size_t buffer_size;
    if( hb_script_json_dumps( L, -1, buffer, HB_DATA_MAX_SIZE, &buffer_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR_INTERNAL_ERROR( L );
    }

    lua_pushlstring( L, buffer, buffer_size );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg globalLib[] = {
    {"print", &__hb_lua_print}
    , {"json_dumps", &__hb_lua_json_dumps}
    , {NULL, NULL} /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg serverLib[] = {
    {"GetProjectPublicData", &hb_script_server_GetProjectPublicData}
    , {"SetProjectPublicData", &hb_script_server_SetProjectPublicData}
    , {"UpdateProjectPublicData", &hb_script_server_UpdateProjectPublicData}
    , {"GetCurrentUserPublicData", &hb_script_server_GetCurrentUserPublicData}
    , {"SetCurrentUserPublicData", &hb_script_server_SetCurrentUserPublicData}
    , {"UpdateCurrentUserPublicData", &hb_script_server_UpdateCurrentUserPublicData}
    , {"GetUserEntityPublicData", &hb_script_server_GetUserEntityPublicData}
    , {"SetUserEntityPublicData", &hb_script_server_SetUserEntityPublicData}
    , {"UpdateCurrentUserPublicData", &hb_script_server_UpdateCurrentUserPublicData}
    , {"CreateUserEntity", &hb_script_server_CreateUserEntity}
    , {"SelectUserEntity", &hb_script_server_SelectUserEntity}
    , {"SetUserEntityPublicData", &hb_script_server_SetUserEntityPublicData}
    , {"GetUserEntityPublicData", &hb_script_server_GetUserEntityPublicData}
    , {"UpdateUserEntityPublicData", &hb_script_server_UpdateUserEntityPublicData}
    , {"CreateProjectEntity", &hb_script_server_CreateProjectEntity}
    , {"SelectProjectEntity", &hb_script_server_SelectProjectEntity}
    , {"GetProjectEntity", &hb_script_server_GetProjectEntity}
    , {"SetUserEntityPublicData", &hb_script_server_SetUserEntityPublicData}
    , {"GetUserEntityPublicData", &hb_script_server_GetUserEntityPublicData}
    , {"UpdateUserEntityPublicData", &hb_script_server_UpdateUserEntityPublicData}
    , {NULL, NULL} /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static const struct luaL_Reg matchingLib[] = {
    {"CreateMatching", &hb_script_server_CreateMatching}
    , {"JoinMatching", &hb_script_server_JoinMatching}
    , {NULL, NULL} /* end of array */
};
//////////////////////////////////////////////////////////////////////////
static int __hb_lua_panic( lua_State * L )
{
    hb_script_handle_t * handle = *(hb_script_handle_t **)lua_getextraspace( L );

    longjmp( handle->panic_jump, 1 );
}
//////////////////////////////////////////////////////////////////////////
static void * __hb_lua_alloc( void * ud, void * ptr, hb_size_t osize, hb_size_t nsize )
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
        HB_FREE( ptr );

        return HB_NULLPTR;
    }
    else
    {
        void * nptr = HB_REALLOC( ptr, nsize );

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
static hb_result_t __hb_script_load( hb_script_handle_t * _handle, const void * _buffer, hb_size_t _size )
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
static hb_result_t __hb_script_load_project( hb_script_handle_t * _handle, hb_uid_t _puid )
{
    const char * db_projects_fields[] = {"script_sha1"};

    hb_db_values_handle_t * project_values;

    if( hb_db_get_values( _handle->db_collection_projects, _puid, db_projects_fields, sizeof( db_projects_fields ) / sizeof( db_projects_fields[0] ), &project_values, HB_NULLPTR ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' not found 'script_sha1'"
            , "projects"
        );

        return HB_FAILURE;
    }

    hb_sha1_t script_sha1;
    if( hb_db_copy_binary_value( project_values, 0, &script_sha1, sizeof( script_sha1 ) ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid data 'script_sha1'"
            , "projects"
        );

        return HB_FAILURE;
    }

    hb_db_destroy_values( project_values );

    hb_size_t script_data_size;
    hb_data_t script_data;
    if( hb_storage_get_code( _handle->cache, _handle->db_collection_scripts, &script_sha1, script_data, sizeof( script_data ), &script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid get data from storage"
            , "projects"
        );

        return HB_FAILURE;
    }

    if( __hb_script_load( _handle, script_data, script_data_size ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "node", "invalid initialize script: collection '%s' invalid load data"
            , "projects"
        );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_script_initialize( const hb_cache_handle_t * _cache, const hb_db_client_handle_t * _db, hb_size_t _memorylimit, hb_size_t _calllimit, hb_uid_t _puid, hb_uid_t _uuid, hb_matching_handle_t * _matching, hb_script_handle_t ** _handle )
{
    hb_script_handle_t * handle = HB_NEW( hb_script_handle_t );

    handle->cache = _cache;
    handle->db_client = _db;

    if( hb_db_get_project_collection( _db, _puid, "user_entities", &handle->db_collection_user_entities ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "user_entities"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "project_entities", &handle->db_collection_project_entities ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "project_entities"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "users", &handle->db_collection_users ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "users"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_collection( _db, "hb", "projects", &handle->db_collection_projects ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "projects"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "matching", &handle->db_collection_matching ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "matching"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "worlds", &handle->db_collection_worlds ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "worlds"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "avatars", &handle->db_collection_avatars ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "avatars"
        );

        return HB_FAILURE;
    }

    if( hb_db_get_project_collection( _db, _puid, "scripts", &handle->db_collection_scripts ) == HB_FAILURE )
    {
        HB_LOG_MESSAGE_ERROR( "script", "invalid initialize script: db not found collection '%s'"
            , "scripts"
        );

        return HB_FAILURE;
    }

    handle->project_uid = _puid;
    handle->user_uid = _uuid;

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

    static const luaL_Reg loadedlibs[] = {
        {LUA_GNAME, luaopen_base},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_UTF8LIBNAME, luaopen_utf8},
        {NULL, NULL}
    };

    for( const luaL_Reg * lib = loadedlibs; lib->func; lib++ )
    {
        luaL_requiref( L, lib->name, lib->func, 1 );
        lua_pop( L, 1 );
    }

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

    if( handle->matching != HB_NULLPTR )
    {
        lua_getglobal( L, "server" );
        luaL_setfuncs( L, matchingLib, 0 );
    }

    handle->call_used = 0;
    handle->call_limit = _calllimit;
    lua_sethook( L, &__hb_lua_hook, LUA_MASKCOUNT, 1 );

    hb_size_t memory_used = handle->memory_used;
    handle->memory_base = memory_used;
    handle->memory_limit = memory_used + _memorylimit;

    handle->L = L;

    *(hb_script_handle_t **)lua_getextraspace( L ) = handle;

    if( __hb_script_load_project( handle, _puid ) == HB_FAILURE )
    {
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
    hb_db_destroy_collection( _handle->db_collection_project_entities );
    hb_db_destroy_collection( _handle->db_collection_user_entities );
    hb_db_destroy_collection( _handle->db_collection_matching );
    hb_db_destroy_collection( _handle->db_collection_worlds );
    hb_db_destroy_collection( _handle->db_collection_avatars );
    hb_db_destroy_collection( _handle->db_collection_scripts );

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
hb_result_t hb_script_api_call( hb_script_handle_t * _handle, const char * _api, const char * _method, const hb_json_handle_t * _json, char * _response, hb_size_t _capacity, hb_size_t * _resultsize, hb_error_code_t * _code )
{
    if( setjmp( _handle->panic_jump ) == 1 )
    {
        /* recovered from panic. log and return */

        return HB_FAILURE;
    }

    HB_LOG_MESSAGE_INFO( "script", "call api '%s' method '%s'"
        , _api
        , _method
    );

    lua_State * L = _handle->L;

    int base = lua_gettop( L );

    lua_getglobal( L, _api );

    if( lua_getfield( L, -1, _method ) != LUA_TFUNCTION )
    {
        *_code = HB_ERROR_NOT_FOUND;

        return HB_SUCCESSFUL;
    }

    lua_remove( L, -2 );

    if( hb_script_json_loads( L, _json ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 1, LUA_MULTRET, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        HB_LOG_MESSAGE_ERROR( "script", "call function '%s' with error: %s"
            , _method
            , error_msg
        );

        *_code = HB_ERROR_INTERNAL;

        return HB_SUCCESSFUL;
    }

    int top = lua_gettop( L );

    int nresults = top - base;

    if( nresults == 1 && lua_type( L, -1 ) == LUA_TTABLE )
    {
        if( _response != HB_NULLPTR )
        {
            if( hb_script_json_dumps( L, -1, _response, _capacity, _resultsize ) == HB_FAILURE )
            {
                return HB_FAILURE;
            }
        }

        lua_pop( L, 1 );
    }
    else if( nresults == 0 )
    {
        if( _response != HB_NULLPTR )
        {
            strcpy( _response, "{}" );

            if( _resultsize != HB_NULLPTR )
            {
                *_resultsize = 2;
            }
        }
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
hb_result_t hb_script_api_call_data( hb_script_handle_t * _handle, const char * _api, const char * _method, const void * _data, hb_size_t _datasize, char * _result, hb_size_t _capacity, hb_size_t * _resultsize, hb_error_code_t * _code )
{
    hb_json_handle_t * json_data;
    if( hb_json_create( _data, _datasize, &json_data ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_result_t result = hb_script_api_call( _handle, _api, _method, json_data, _result, _capacity, _resultsize, _code );

    hb_json_destroy( json_data );

    return result;
}