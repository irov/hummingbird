#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_SetCurrentUserPublicData( lua_State * L )
{
    char json_data[2048];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 1, json_data, 2048, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_value_handle_t handler[1];
    hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size, handler + 0 );

    if( hb_db_update_values( g_script_handle->db_collection_users, g_script_handle->user_oid, handler, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    return 0;
}