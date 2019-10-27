#include "hb_script_handle.h"

#include "hb_json/hb_json.h"

//////////////////////////////////////////////////////////////////////////
extern struct hb_script_handle_t * g_script_handle;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_GetCurrentUserPublicData( lua_State * L )
{
    const char * fields[16];

    uint32_t field_iterator = 0;

    lua_pushnil( L );
    while( lua_next( L, 1 ) != 0 )
    {
        const char * value = lua_tostring( L, -1 );
        fields[field_iterator++] = value;

        lua_pop( L, 1 );
    }

    const char * db_fields[1] = { "public_data" };

    hb_db_value_handle_t handler[1];
    if( hb_db_get_values( g_script_handle->db_user_collection, g_script_handle->user_oid, db_fields, 1, handler ) == HB_FAILURE )
    {
        lua_pushboolean( L, 0 );

        for( uint32_t index = 0; index != field_iterator; ++index )
        {
            lua_pushnil( L );
        }

        return 1 + field_iterator;
    }

    lua_pushboolean( L, 1 );

    hb_json_handle_t json_data;
    hb_json_create( handler[0].u.utf8.buffer, handler[0].u.utf8.length, &json_data );

    for( uint32_t index = 0; index != field_iterator; ++index )
    {
        const char * field = fields[index];

        hb_json_handle_t json_field;
        hb_json_get_field( &json_data, field, &json_field );

        hb_json_type_t json_field_type = hb_json_get_type( &json_field );

        switch( json_field_type )
        {
        case e_hb_json_object:
            {

            }break;
        case e_hb_json_array:
            {

            }break;
        case e_hb_json_string:
            {
                size_t length;
                const char * value;
                hb_json_to_string( &json_field, &value, &length );

                lua_pushlstring( L, value, length );
            }break;
        case e_hb_json_integer:
            {

            }break;
        case e_hb_json_real:
            {

            }break;
        case e_hb_json_true:
            {

            }break;
        case e_hb_json_false:
            {

            }break;
        case e_hb_json_null:
            {

            }break;
        default:
            break;
        }
    }

    hb_db_destroy_values( handler, 1 );

    return 1 + field_iterator;
}