#include "hb_script_handle.h"
#include "hb_script_json.h"

#include "hb_matching/hb_matching.h"
#include "hb_json/hb_json.h"
#include "hb_utils/hb_rand.h"
#include "hb_utils/hb_base16.h"

#include <string.h>

int hb_script_server_CreateMatching( lua_State * L )
{
    hb_script_handle_t * script_handle = *(hb_script_handle_t **)lua_getextraspace( L );

    hb_size_t name_len;
    const char * name = lua_tolstring( L, 1, &name_len );
    lua_Integer count = lua_tointegerx( L, 2, HB_NULLPTR );
    lua_Integer dispersion = lua_tointegerx( L, 3, HB_NULLPTR );

    char json_data[HB_DATA_MAX_SIZE];
    hb_size_t json_data_size;
    if( hb_script_json_dumps( L, 4, json_data, HB_DATA_MAX_SIZE, &json_data_size ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR_INTERNAL_ERROR( L );
    }

    hb_matching_desc_t desc;
    desc.matching_count = (uint32_t)count;
    desc.matching_dispersion = (uint32_t)dispersion;
    desc.join_timeout = 300000;
    desc.wait_timeout = 10000;

    hb_bool_t exist;
    if( hb_matching_room_create( script_handle->matching, script_handle->db_client, script_handle->project_uid, name, name_len, &desc, json_data, json_data_size, &exist ) == HB_FAILURE )
    {
        HB_SCRIPT_ERROR_INTERNAL_ERROR( L );
    }

    if( exist == HB_TRUE )
    {
        HB_SCRIPT_ERROR( L, "game exist game" );
    }

    return 0;
}