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
int __hb_script_server_GetProjectEntityPublicData( lua_State * L )
{
    lua_Integer eid = lua_tointeger( L, 1 );

    const char * fields[16];
    uint32_t field_iterator = 0;

    lua_pushvalue( L, 2 );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 )
    {
        if( field_iterator == 16 )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        const char * value = lua_tostring( L, -1 );
        fields[field_iterator++] = value;

        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

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

        for( uint32_t index = 0; index != field_iterator; ++index )
        {
            lua_pushnil( L );
        }

        return 1 + field_iterator;
    }

    const char * db_fields[1] = {"public_data"};

    hb_db_value_handle_t handler[1];
    if( hb_db_get_values( g_script_handle->db_collection_project_entities, eoid, db_fields, handler, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushboolean( L, 1 );

    if( hb_script_json_load_fields( L, handler[0].u.symbol.buffer, handler[0].u.symbol.length, fields, field_iterator ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( handler, 1 );

    return 1 + field_iterator;
}