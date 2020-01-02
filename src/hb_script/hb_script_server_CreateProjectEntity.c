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
int __hb_script_server_CreateProjectEntity( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    size_t parent_len;
    const char * parent = lua_tolstring( L, 2, &parent_len );

    char json_data[2048];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 3, json_data, 2048, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_oid_t eoid;
    if( name_len != 0 && parent_len == 0 )
    {
        hb_db_value_handle_t find_values[2];
        hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, find_values + 0 );
        hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, name, name_len, find_values + 1 );

        hb_bool_t exist;
        if( hb_db_find_oid( g_script_handle->db_collection_project_entities, find_values, 2, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        if( exist == HB_TRUE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_value_handle_t new_values[4];
        hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, name, name_len, new_values + 0 );
        hb_db_make_symbol_value( "parent", HB_UNKNOWN_STRING_SIZE, parent, parent_len, new_values + 1 );
        hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, new_values + 2 );
        hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size, new_values + 3 );
        
        if( hb_db_new_document( g_script_handle->db_collection_project_entities, new_values, 4, &eoid ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }
    }
    else if( name_len == 0 && parent_len != 0 )
    {
        hb_db_value_handle_t find_values[2];
        hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, find_values + 0 );
        hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, parent, parent_len, find_values + 1 );

        hb_bool_t exist;
        if( hb_db_find_oid( g_script_handle->db_collection_project_entities, find_values, 2, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        if( exist == HB_FALSE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_value_handle_t new_values[3];
        hb_db_make_symbol_value( "parent", HB_UNKNOWN_STRING_SIZE, parent, parent_len, new_values + 0 );
        hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, new_values + 1 );
        hb_db_make_symbol_value( "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size, new_values + 2 );

        if( hb_db_new_document( g_script_handle->db_collection_project_entities, new_values, 4, &eoid ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }
    }
    else
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_value_handle_t count_values[1];
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, count_values + 0 );

    hb_pid_t pid;
    if( hb_db_make_pid( g_script_handle->db_collection_user_entities, eoid, count_values, 1, &pid ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushinteger( L, pid );

    return 1;
}