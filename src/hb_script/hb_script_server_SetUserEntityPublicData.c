#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_SetUserEntityPublicData( lua_State * L )
{
    lua_Integer eid = lua_tointeger( L, 1 );

    hb_db_value_handle_t values[3];
    hb_db_make_int32_value( "eid", ~0U, (int32_t)eid, values + 0 );
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, values + 1 );
    hb_db_make_oid_value( "uoid", ~0U, g_script_handle->user_oid, values + 2 );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( g_script_handle->db_collection_entities, values, 3, &eoid, &exist ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    if( exist == HB_FALSE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    char json_data[2048];
    size_t json_data_size;
    hb_script_json_dumps( L, 2, json_data, 2048, &json_data_size );

    hb_db_value_handle_t handler[1];
    hb_db_make_symbol_value( "public_data", ~0U, json_data, json_data_size, handler + 0 );

    if( hb_db_update_values( g_script_handle->db_collection_entities, eoid, handler, 1 ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    lua_pushboolean( L, 1 );

    return 1;
}