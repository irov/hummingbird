#include "hb_grid_process_setusernickname.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

hb_result_t hb_grid_process_setusernickname( hb_grid_process_handle_t * _process, const hb_grid_process_setusernickname_in_data_t * _in, hb_grid_process_setusernickname_out_data_t * _out )
{
    HB_UNUSED( _out );

    if( hb_cache_expire_value( _process->cache, _in->token.value, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_user_token_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token.value, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_values_handle_t * values_update;
    if( hb_db_create_values( &values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_make_symbol_value( values_update, "info_nickname", HB_UNKNOWN_STRING_SIZE, _in->nickname, HB_UNKNOWN_STRING_SIZE );

    if( hb_db_update_values_by_name( _process->db_client, "hb_users", &token_handle.uoid, values_update ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_db_destroy_values( values_update );

    return HB_SUCCESSFUL;
}