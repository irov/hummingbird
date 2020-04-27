#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

int __hb_script_server_SetUserEntityPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer pid = lua_tointeger( L, 1 );

    hb_db_values_handle_t * values;

    if( hb_db_create_values( &values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_int32_value( values, "pid", HB_UNKNOWN_STRING_SIZE, (int32_t)pid );
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

    char json_data[2048];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 2, json_data, 2048, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_values_handle_t * handler;

    if( hb_db_create_values( &handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_symbol_value( handler, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );

    if( hb_db_update_values( script_handle->db_collection_user_entities, eoid, handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( handler );

    return 0;
}