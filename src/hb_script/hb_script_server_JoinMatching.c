#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
static hb_result_t __hb_matching_complete( const hb_matching_user_t * _user, int32_t _count, const char * _data, size_t _datasize, void * _ud )
{
    lua_State * L = (lua_State *)_ud;

    lua_getglobal( L, "event" );

    if( lua_getfield( L, -1, "onCreateWorld" ) != LUA_TFUNCTION )
    {
        return HB_SUCCESSFUL;
    }

    HB_UNUSED( _user );
    HB_UNUSED( _count );
    HB_UNUSED( _data );
    HB_UNUSED( _datasize );
    HB_UNUSED( _ud );

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