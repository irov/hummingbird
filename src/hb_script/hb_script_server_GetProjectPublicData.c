#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

int __hb_script_server_GetProjectPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

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

    hb_db_values_handle_t * project_values;
    if( hb_db_get_values( script_handle->db_collection_projects, script_handle->project_oid, db_fields, 1, &project_values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    const char * public_data_symbol;
    size_t public_data_symbol_length;

    if( hb_db_get_symbol_value( project_values, 0, &public_data_symbol, &public_data_symbol_length ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( hb_script_json_load_fields( L, public_data_symbol, public_data_symbol_length, fields, field_iterator ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( project_values );

    return field_iterator;
}