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
    size_t parent_len;
    const char * parent = lua_tolstring( L, 1, &parent_len );

    lua_Integer limit = lua_tointeger( L, 2 );

    if( parent_len == 0 )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( limit < 0 )
    {
        limit = 32;
    }
    else if( limit > 32 )
    {
        HB_SCRIPT_ERROR( L, "limit max 32" );
    }

    hb_db_value_handle_t query[3];
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, parent, parent_len, query + 0 );
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, query + 1 );
    hb_db_make_oid_value( "uoid", HB_UNKNOWN_STRING_SIZE, g_script_handle->user_oid, query + 2 );    

    const char * db_fields[1] = {"eid"};

    uint32_t exists = 0;
    hb_db_value_handle_t values[1 * 32];
    if( hb_db_select_values( g_script_handle->db_collection_user_entities, query, 3, db_fields, 1, values, (uint32_t)limit, &exists ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
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