#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_json/hb_json.h"

#ifndef HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX
#define HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX 16
#endif

int __hb_script_server_GetCurrentUserPublicData( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    const char * db_fields[1] = {"public_data"};

    hb_db_values_handle_t * user_values;
    if( hb_db_get_values( script_handle->db_collection_users, script_handle->user_oid, db_fields, 1, &user_values ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    const char * fields[HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX];

    uint32_t field_iterator = 0;

    lua_pushvalue( L, 1 );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 )
    {
        if( field_iterator == HB_SCRIPT_USER_PUBLIC_DATA_FIELD_MAX )
        {
            HB_SCRIPT_ERROR( L, "internal error" );
        }

        const char * value = lua_tostring( L, -1 );
        fields[field_iterator++] = value;

        lua_pop( L, 1 );
    }
    lua_pop( L, 1 );

    const char * public_data_symbol;
    size_t public_data_symbol_length;

    if( hb_db_get_symbol_value( user_values, 0, &public_data_symbol, &public_data_symbol_length ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( hb_script_json_load_fields( L, public_data_symbol, public_data_symbol_length, fields, field_iterator ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_destroy_values( user_values );

    return field_iterator;
}