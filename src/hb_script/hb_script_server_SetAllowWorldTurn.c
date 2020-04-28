#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int __hb_script_server_SetAllowWorldTurn( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer pid = lua_tointeger( L, 1 );

    hb_db_values_handle_t * values_found;
    if( hb_db_create_values( &values_found ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_int32_value( values_found, "pid", HB_UNKNOWN_STRING_SIZE, (int32_t)pid );
    hb_db_make_oid_value( values_found, "poid", HB_UNKNOWN_STRING_SIZE, script_handle->project_oid );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( script_handle->db_collection_project_entities, values_found, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( values_found );

    if( exist == HB_FALSE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    char json_data[HB_DATA_MAX_SIZE];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 2, json_data, HB_DATA_MAX_SIZE, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_values_handle_t * values_update;
    if( hb_db_create_values( &values_update ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_symbol_value( values_update, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );

    if( hb_db_update_values( script_handle->db_collection_project_entities, eoid, values_update ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( values_update );

    return 0;
}