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
int __hb_script_server_JoinMatching( lua_State * L )
{
    size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );

    //lua_Integer dispersion = lua_tointegerx( L, 2, HB_NULLPTR );

    hb_db_value_handle_t query[2];
    hb_db_make_oid_value( "poid", HB_UNKNOWN_STRING_SIZE, g_script_handle->project_oid, query + 0 );
    hb_db_make_symbol_value( "name", HB_UNKNOWN_STRING_SIZE, name, name_len, query + 1 );

    hb_bool_t exist;
    hb_oid_t moid;
    if( hb_db_find_oid( g_script_handle->db_collection_matching, query, 2, &moid, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR( L, "internal error" );
    }

    if( exist == HB_FALSE )
    {
        lua_pushboolean( L, 0 );

        return 1;
    }

    //ToDo

    return 1;
}