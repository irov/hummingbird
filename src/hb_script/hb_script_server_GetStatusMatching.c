#include "hb_script_handle.h"

#include "hb_matching/hb_matching.h"

int __hb_script_server_GetStatusMatching( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    hb_size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    hb_bool_t exist;
    hb_uid_t auid;
    if( hb_matching_found( script_handle->matching, script_handle->db_client, script_handle->project_uid, name, name_len, script_handle->user_uid, &exist, &auid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR_INTERNAL_ERROR( L );
    }

    if( exist == HB_FALSE )
    {
        lua_pushboolean( L, 0 );
        lua_pushinteger( L, 0 );
    }
    else
    {
        lua_pushboolean( L, 1 );
        lua_pushinteger( L, auid );
    }

    return 2;
}