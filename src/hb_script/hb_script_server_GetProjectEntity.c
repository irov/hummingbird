#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_oid.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_script_server_GetProjectEntity( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    if( name_len == 0 )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_values_handle_t * query_values;
    if( hb_db_create_values( &query_values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_make_symbol_value( query_values, "name", HB_UNKNOWN_STRING_SIZE, name, name_len );
    hb_db_make_oid_value( query_values, "poid", HB_UNKNOWN_STRING_SIZE, &script_handle->project_oid );

    hb_bool_t exist;
    hb_oid_t eoid;
    if( hb_db_find_oid( script_handle->db_collection_project_entities, query_values, &eoid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( query_values );

    const char * db_fields[1] = { "uid" };

    hb_db_values_handle_t * project_entity_values;
    if( hb_db_get_values( script_handle->db_collection_project_entities, &eoid, db_fields, 1, &project_entity_values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_uid_t uid;
    if( hb_db_get_uid_value( project_entity_values, 0, &uid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( project_entity_values );

    lua_pushinteger( L, uid );

    return 1;
}