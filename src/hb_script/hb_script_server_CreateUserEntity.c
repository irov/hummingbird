#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_CreateUserEntity( lua_State * L )
{
    char json_data[2048];
    size_t json_data_size;
    hb_script_json_dumps( L, 1, json_data, 2048, &json_data_size );

    hb_db_value_handle_t values[3];
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, values + 0 );
    hb_db_make_oid_value( "uoid", ~0U, g_script_handle->user_oid, values + 1 );
    hb_db_make_symbol_value( "public_data", ~0U, json_data, json_data_size, values + 2 );

    hb_oid_t eoid;
    if( hb_db_new_document( g_script_handle->db_entities_collection, values, 3, &eoid ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );
        lua_pushnil( L );

        return 2;
    }

    lua_pushboolean( L, 1 );

    hb_oid16_t eoid16;
    hb_oid_base16_encode( eoid, &eoid16 );

    lua_pushlstring( L, eoid16, sizeof( hb_oid16_t ) );

    return 1;
}