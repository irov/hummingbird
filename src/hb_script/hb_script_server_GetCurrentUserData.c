#include "hb_script_handler.h"

//////////////////////////////////////////////////////////////////////////
extern struct hb_script_handler_t * g_script_handler;
//////////////////////////////////////////////////////////////////////////
int __hb_script_server_GetCurrentUserData( lua_State * L )
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

    hb_db_value_handler_t handler;
    if( hb_db_get_value( &g_script_handler->db_collection, g_script_handler->user, fields, field_iterator, &handler ) == 0 )
    {
        lua_pushboolean( L, 0 );

        for( uint32_t index = 0; index != field_iterator; ++index )
        {
            lua_pushnil( L );
        }

        return 1 + field_iterator;
    }

    lua_pushboolean( L, 1 );

    for( uint32_t index = 0; index != field_iterator; ++index )
    {
        const char * value = handler.value[index];
        size_t length = handler.length[index];

        lua_pushlstring( L, value, length );
    }

    hb_db_value_destroy( &handler );

    return 1 + field_iterator;
}