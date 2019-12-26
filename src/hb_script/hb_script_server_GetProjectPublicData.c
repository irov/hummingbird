#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_GetProjectPublicData( lua_State * L )
{
    const char * fields[16];

    uint32_t field_iterator = 0;

    lua_pushvalue( L, 1 );
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

    const char * db_fields[1] = {"public_data"};

    hb_db_value_handle_t project_values[1];
    if( hb_db_get_values( g_script_handle->db_collection_projects, g_script_handle->project_oid, db_fields, project_values, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushboolean( L, 1 );

    if( hb_script_json_load_fields( L, project_values[0].u.symbol.buffer, project_values[0].u.symbol.length, fields, field_iterator ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( project_values, 1 );

    return 1 + field_iterator;
}