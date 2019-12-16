#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_UpdateCurrentUserPublicData( lua_State * L )
{
    const char * db_fields[1] = { "public_data" };

    hb_db_value_handle_t handler[1];
    if( hb_db_get_values( g_script_handle->db_collection_users, g_script_handle->user_oid, db_fields, handler, 1 ) == HB_FAILURE )
    {
        return -1;
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( handler[0].u.symbol.buffer, handler[0].u.symbol.length, &json_data ) == HB_FAILURE )
    {
        return -1;
    }

    hb_json_handle_t * json_update;
    if( hb_script_json_create( L, 1, &json_update ) == HB_FAILURE )
    {
        lua_pushstring( L, "incorrect argument json update" );
        lua_error( L );

        return 0;
    }

    if( hb_json_update( json_data, json_update ) == HB_FAILURE )
    {
        return -1;
    }

    char json_new_data[10240];
    size_t json_new_data_size;
    if( hb_json_dumps( json_data, json_new_data, 10240, &json_new_data_size ) == HB_FAILURE )
    {
        return -1;
    }

    hb_db_value_handle_t update_handler[1];
    hb_db_make_symbol_value( "public_data", ~0U, json_new_data, json_new_data_size, update_handler + 0 );

    if( hb_db_update_values( g_script_handle->db_collection_users, g_script_handle->user_oid, update_handler, 1 ) == HB_FAILURE )
    {
        return -1;
    }

    lua_pushboolean( L, 1 );

    return 1;
}