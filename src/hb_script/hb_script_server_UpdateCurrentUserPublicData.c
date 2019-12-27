#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_UpdateCurrentUserPublicData( lua_State * L )
{
    const char * db_fields[1] = {"public_data"};

    hb_db_value_handle_t user_values[1];
    if( hb_db_get_values( g_script_handle->db_collection_users, g_script_handle->user_oid, db_fields, user_values, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( user_values[0].u.symbol.buffer, user_values[0].u.symbol.length, &json_data ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( user_values, 1 );

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

    hb_db_value_handle_t update_handler[1];
    hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, json_new_data, json_new_data_size, update_handler + 0 );

    if( hb_db_update_values( g_script_handle->db_collection_users, g_script_handle->user_oid, update_handler, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}