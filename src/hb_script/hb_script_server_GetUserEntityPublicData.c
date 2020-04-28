#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

int __hb_script_server_GetUserEntityPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer eid = lua_tointeger( L, 1 );

    hb_db_values_handle_t * values;

    if( hb_db_create_values( &values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_int32_value( values, "pid", HB_UNKNOWN_STRING_SIZE, (int32_t)eid );
    hb_db_make_oid_value( values, "poid", HB_UNKNOWN_STRING_SIZE, script_handle->project_oid );
    hb_db_make_oid_value( values, "uoid", HB_UNKNOWN_STRING_SIZE, script_handle->user_oid );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( script_handle->db_collection_user_entities, values, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( values );

    if( exist == HB_FALSE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    uint32_t fields_count;
    if( hb_script_json_get_public_data( L, 2, script_handle->db_collection_user_entities, eoid, &fields_count ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return fields_count;
}