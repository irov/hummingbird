#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_log/hb_log.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_matching_complete( const hb_matching_complete_desc_t * _desc )
{
    lua_State * L = (lua_State *)_desc->ud;

    lua_getglobal( L, "event" );

    if( lua_getfield( L, -1, "onCreateWorld" ) != LUA_TFUNCTION )
    {
        return HB_SUCCESSFUL;
    }

    lua_pushstring( L, _desc->name );
    lua_pushinteger( L, _desc->wpid );

    lua_createtable( L, _desc->users_count, 0 );
    for( uint32_t index = 0; index != _desc->users_count; ++index )
    {
        const hb_matching_user_t * user = _desc->users + index;

        lua_pushinteger( L, user->apid );
        lua_rawseti( L, -2, index + 1 );
    }

    if( hb_script_json_loads( L, _desc->data, _desc->data_size ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    int status = lua_pcallk( L, 4, 0, 0, 0, HB_NULLPTR );

    if( status != LUA_OK )
    {
        const char * error_msg = lua_tolstring( L, -1, HB_NULLPTR );

        HB_LOG_MESSAGE_ERROR( "script", "call function '%s' data '%.*s' with error: %s"
            , "onCreateWorld"
            , _desc->data_size
            , _desc->data
            , error_msg
        );

        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_JoinMatching( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );
    lua_Integer rating = lua_tointegerx( L, 2, HB_NULLPTR );

    hb_bool_t exist;
    if( hb_matching_join( g_script_handle->matching, g_script_handle->project_oid, name, name_len, g_script_handle->user_oid, (int32_t)rating, &exist, &__hb_matching_complete, (void *)L ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( exist == HB_FALSE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}