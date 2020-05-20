#include "hb_script_handle.h"
#include "hb_script_json.h"

int hb_script_server_SetCurrentUserPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    if( hb_script_json_set_public_data( L, 1, script_handle->db_collection_users, &script_handle->user_oid ) == HB_FAILURE )
    { 
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}