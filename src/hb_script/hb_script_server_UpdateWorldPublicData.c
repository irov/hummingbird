#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int __hb_script_server_UpdateWorldPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    lua_Integer pid = lua_tointeger( L, 1 );

    hb_db_value_handle_t values[2];
    hb_db_make_int32_value( "pid", HB_UNKNOWN_STRING_SIZE, (int32_t)pid, values + 0 );
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, script_handle->project_oid, values + 1 );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( script_handle->db_collection_project_entities, values, 2, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( exist == HB_FALSE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    const char * db_fields[1] = {"public_data"};

    hb_db_value_handle_t entity_get_values[1];
    if( hb_db_get_values( script_handle->db_collection_project_entities, eoid, db_fields, entity_get_values, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( entity_get_values[0].u.symbol.buffer, entity_get_values[0].u.symbol.length, &json_data ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( entity_get_values, 1 );

    hb_json_handle_t * json_update;
    if( hb_script_json_create( L, 1, &json_update ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( hb_json_update( json_data, json_update ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    char json_new_data[10240];
    size_t json_new_data_size;
    if( hb_json_dumps( json_data, json_new_data, 10240, &json_new_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_value_handle_t entity_update_values[1];
    hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, json_new_data, json_new_data_size, entity_update_values + 0 );

    if( hb_db_update_values( script_handle->db_collection_project_entities, eoid, entity_update_values, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( entity_update_values, 1 );

    return 0;
}