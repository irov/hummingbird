#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int __hb_script_server_SetAllowWorldTurn( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer uid = lua_tointeger( L, 1 );

    hb_db_values_handle_t * values_found;
    if( hb_db_create_values( &values_found ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_uid_value( values_found, "_id", HB_UNKNOWN_STRING_SIZE, (hb_uid_t)uid );
    hb_db_make_uid_value( values_found, "poid", HB_UNKNOWN_STRING_SIZE, script_handle->project_oid );

    hb_bool_t exist;
    hb_uid_t eoid;
    if( hb_db_find_oid( script_handle->db_collection_project_entities, values_found, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( values_found );

    if( exist == HB_FALSE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( hb_script_json_set_public_data( L, 2, script_handle->db_collection_project_entities, eoid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}