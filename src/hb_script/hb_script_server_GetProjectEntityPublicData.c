#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_script_server_GetProjectEntityPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer euid = lua_tointeger( L, 1 );

    uint32_t fields_count;
    if( hb_script_json_get_public_data( L, 2, script_handle->db_collection_project_entities, (hb_uid_t)euid, &fields_count ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return fields_count;
}