#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_script_server_CreateProjectEntity( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    size_t parent_len;
    const char * parent = lua_tolstring( L, 2, &parent_len );

    char json_data[HB_DATA_MAX_SIZE];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 3, json_data, HB_DATA_MAX_SIZE, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_uid_t eoid;
    if( name_len != 0 && parent_len == 0 )
    {
        hb_db_values_handle_t * find_values;
        if( hb_db_create_values( &find_values ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_make_string_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, name, name_len );

        hb_bool_t exist;
        if( hb_db_find_uid( script_handle->db_collection_project_entities, find_values, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_destroy_values( find_values );

        if( exist == HB_TRUE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_values_handle_t * new_values;
        if( hb_db_create_values( &new_values ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_make_string_value( new_values, "name", HB_UNKNOWN_STRING_SIZE, name, name_len );
        hb_db_make_string_value( new_values, "parent", HB_UNKNOWN_STRING_SIZE, parent, parent_len );
        hb_db_make_string_value( new_values, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );
        
        if( hb_db_new_document( script_handle->db_collection_project_entities, new_values, &eoid ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_destroy_values( new_values );
    }
    else if( name_len == 0 && parent_len != 0 )
    {
        hb_db_values_handle_t * find_values;
        if( hb_db_create_values( &find_values ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_make_string_value( find_values, "name", HB_UNKNOWN_STRING_SIZE, parent, parent_len );

        hb_bool_t exist;
        if( hb_db_find_uid( script_handle->db_collection_project_entities, find_values, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_destroy_values( find_values );

        if( exist == HB_FALSE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_values_handle_t * new_values;
        if( hb_db_create_values( &new_values ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_make_string_value( new_values, "parent", HB_UNKNOWN_STRING_SIZE, parent, parent_len );
        hb_db_make_string_value( new_values, "public_data", HB_UNKNOWN_STRING_SIZE, json_data, json_data_size );

        if( hb_db_new_document( script_handle->db_collection_project_entities, new_values, &eoid ) == HB_FAILURE )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        hb_db_destroy_values( new_values );
    }
    else
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushinteger( L, eoid );

    return 1;
}