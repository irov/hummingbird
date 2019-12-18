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
int __hb_script_server_CreateMatching( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );
    lua_Integer count = lua_tointegerx( L, 2, HB_NULLPTR );
    lua_Integer dispersion = lua_tointegerx( L, 3, HB_NULLPTR );

    char json_data[2048];
    size_t json_data_size;
    if( hb_script_json_dumps( L, 4, json_data, 2048, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    hb_db_value_handle_t find_values[2];
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, find_values + 0 );
    hb_db_make_symbol_value( "name", ~0U, name, name_len, find_values + 1 );

    hb_bool_t exist;
    if( hb_db_find_oid( g_script_handle->db_collection_matching, find_values, 2, HB_NULLPTR, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( exist == HB_TRUE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    hb_db_value_handle_t new_candidate_values[5];
    hb_db_make_oid_value( "poid", ~0U, g_script_handle->project_oid, new_candidate_values + 0 );
    hb_db_make_symbol_value( "name", ~0U, name, name_len, new_candidate_values + 1 );
    hb_db_make_int32_value( "count", ~0U, (int32_t)count, new_candidate_values + 2 );
    hb_db_make_int32_value( "dispersion", ~0U, (int32_t)dispersion, new_candidate_values + 3 );
    hb_db_make_symbol_value( "public_data", ~0U, json_data, json_data_size, new_candidate_values + 4 );

    if( hb_db_new_document( g_script_handle->db_collection_matching, new_candidate_values, 5, HB_NULLPTR ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    lua_pushboolean( L, 1 );

    return 1;
}