#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

int __hb_script_server_SetCurrentUserPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    char json_data[2048];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 1, json_data, 2048, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_values_handle_t * handler;

    if( hb_db_create_values( &handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_symbol_value( handler, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );

    if( hb_db_update_values( script_handle->db_collection_users, script_handle->user_oid, handler ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( handler );

    return 0;
}