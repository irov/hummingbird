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
int __hb_script_server_CreateUserEntity( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    size_t parent_len;
    const char * parent = lua_tolstring( L, 2, &parent_len );

    char json_data[2048];
    size_t json_data_size;
    hb_script_json_dumps( L, 3, json_data, 2048, &json_data_size );

    uint32_t eid = 0;

    for( ;; )
    {
        eid = hb_rand_time();
        eid &= 0x7fffffff;

        hb_db_value_handle_t values[4];
        hb_db_make_int32_value( "eid", ~0U, eid, values + 0 );
        hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, values + 1 );
        hb_db_make_oid_value( "uoid", ~0U, g_script_handle->user_oid, values + 2 );

        uint32_t extra_values = 0;       

        if( name_len != 0 )
        {
            hb_db_make_symbol_value( "name", ~0U, name, name_len, values + 3 );

            extra_values += 1;
        }

        hb_bool_t exist;
        if( hb_db_find_oid( g_script_handle->db_collection_user_entities, values, 3 + extra_values, HB_NULLPTR, &exist ) == HB_FAILURE )
        {
            return -1;
        }

        if( exist == HB_FALSE )
        {
            break;
        }
    }

    hb_db_value_handle_t values[6];
    hb_db_make_int32_value( "eid", ~0U, eid, values + 0 );
    hb_db_make_symbol_value( "name", ~0U, name, name_len, values + 1 );
    hb_db_make_symbol_value( "parent", ~0U, parent, parent_len, values + 2 );
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, values + 3 );
    hb_db_make_oid_value( "uoid", ~0U, g_script_handle->user_oid, values + 4 );
    hb_db_make_symbol_value( "public_data", ~0U, json_data, json_data_size, values + 5 );

    hb_oid_t eoid;
    if( hb_db_new_document( g_script_handle->db_collection_user_entities, values, 6, &eoid ) == HB_FAILURE )
    {
        return -1;
    }

    lua_pushboolean( L, 1 );
    lua_pushinteger( L, eid );

    return 2;
}