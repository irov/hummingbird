#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"

//////////////////////////////////////////////////////////////////////////
extern hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_GetUserEntityPublicData( lua_State * L )
{
    const char * entity = lua_tostring( L, 1 );

    hb_oid_t eoid;
    hb_oid_make( entity, &eoid );

    const char * fields[16];
    uint32_t field_iterator = 0;

    lua_pushvalue( L, 2 );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 )
    {
        const char * value = lua_tostring( L, -1 );
        fields[field_iterator++] = value;

        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

    const char * db_fields[1] = { "public_data" };

    hb_db_value_handle_t handler[1];
    if( hb_db_get_values( g_script_handle->db_entities_collection, eoid, db_fields, 1, handler ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );

        for( uint32_t index = 0; index != field_iterator; ++index )
        {
            lua_pushnil( L );
        }

        return 1 + field_iterator;
    }

    lua_pushboolean( L, 1 );

    if( hb_script_json_load_fields( L, handler[0].u.symbol.buffer, handler[0].u.symbol.length, fields, field_iterator ) == HB_FAILURE )
    {
        return -1;
    }

    hb_db_destroy_values( handler, 1 );

    return 1 + field_iterator;
}