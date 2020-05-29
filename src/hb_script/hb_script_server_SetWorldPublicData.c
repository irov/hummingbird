#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int __hb_script_server_SetWorldPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer uid = lua_tointeger( L, 1 );

    if( hb_script_json_set_public_data( L, 2, script_handle->db_collection_project_entities, (hb_uid_t)uid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}