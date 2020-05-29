#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

int hb_script_server_SetProjectPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    if( hb_script_json_set_public_data( L, 1, script_handle->db_collection_projects, script_handle->project_uid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}