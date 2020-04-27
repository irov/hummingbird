#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

int __hb_script_server_UpdateUserEntityPublicData( lua_State * L )
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

    const char * db_fields[1] = {"public_data"};

    hb_db_values_handle_t * user_entity_values;
    if( hb_db_get_values( script_handle->db_collection_user_entities, eoid, db_fields, 1, &user_entity_values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    const char * public_data_symbol;
    size_t public_data_symbol_length;

    if( hb_db_get_symbol_value( user_entity_values, 0, &public_data_symbol, &public_data_symbol_length ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( public_data_symbol, public_data_symbol_length, &json_data ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( user_entity_values );

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

    hb_db_values_handle_t * update_handler;

    if( hb_db_create_values( &update_handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_symbol_value( update_handler, "public_data", HB_UNKNOWN_STRING_SIZE, json_new_data, json_new_data_size );

    if( hb_db_update_values( script_handle->db_collection_user_entities, eoid, update_handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( update_handler );

    return 0;
}