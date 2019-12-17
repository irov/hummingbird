#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_UpdateProjectEntityPublicData( lua_State * L )
{
    lua_Integer eid = lua_tointeger( L, 1 );

    hb_db_value_handle_t values[2];
    hb_db_make_int32_value( "eid", ~0U, (int32_t)eid, values + 0 );
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, values + 1 );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( g_script_handle->db_collection_project_entities, values, 2, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( exist == HB_FALSE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    const char * db_fields[1] = { "public_data" };

    hb_db_value_handle_t handler[1];
    if( hb_db_get_values( g_script_handle->db_collection_project_entities, eoid, db_fields, handler, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_json_handle_t * json_data;
    if( hb_json_create( handler[0].u.symbol.buffer, handler[0].u.symbol.length, &json_data ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

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
    hb_db_make_symbol_value( "public_data", ~0U, json_new_data, json_new_data_size, update_handler + 0 );

    if( hb_db_update_values( g_script_handle->db_collection_project_entities, eoid, update_handler, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushboolean( L, 1 );

    return 1;
}