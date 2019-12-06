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
int __hb_script_server_SelectUserEntity( lua_State * L )
{
    const char * parent = lua_tostring( L, 1 );

    if( strlen( parent ) == 0 )
    {
        lua_pushboolean( L, 1 );
        lua_createtable( L, 0, 0 );

        return 2;
    }

    hb_db_value_handle_t query[3];
    hb_db_make_symbol_value( "name", ~0U, parent, ~0U, query + 0 );
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, query + 1 );
    hb_db_make_oid_value( "uoid", ~0U, g_script_handle->user_oid, query + 2 );    

    const char * db_fields[1] = {"eid"};

    uint32_t exists = 0;
    hb_db_value_handle_t values[1 * 32];
    if( hb_db_select_values( g_script_handle->db_collection_user_entities, query, 3, db_fields, 1, values, 32, &exists ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );
        lua_pushnil( L );

        return 2;
    }

    lua_pushboolean( L, 1 );
    lua_createtable( L, exists, 0 );
    for( uint32_t index = 0; index != exists; ++index )
    {
        hb_db_value_handle_t * value = values + index * 1 + 0;

        lua_pushinteger( L, value->u.i32 );
        lua_rawseti( L, -2, index + 1 );
    }

    hb_db_destroy_values( values, 1 * exists );

    return 2;
}