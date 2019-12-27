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
int __hb_script_server_GetProjectEntity( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    if( name_len == 0 )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_value_handle_t query_values[2];
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, name, name_len, query_values + 0 );
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, query_values + 1 );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( g_script_handle->db_collection_project_entities, query_values, 2, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    const char * db_fields[1] = {"eid"};

    hb_db_value_handle_t project_entity_values[1];
    if( hb_db_get_values( g_script_handle->db_collection_project_entities, eoid, db_fields, project_entity_values, 1 ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushinteger( L, project_entity_values[0].u.i32 );

    hb_db_destroy_values( project_entity_values, 1 );

    return 1;
}