#include "hb_grid_process_setleaderscore.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

hb_result_t hb_grid_process_setleadscore( hb_grid_process_handle_t * _process, const hb_grid_process_setleaderscore_in_data_t * _in, hb_grid_process_setleaderscore_out_data_t * _out )
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

    if( hb_leaderboard_set( _process->db_client, _process->cache, token_handle.puid, token_handle.uuid, _in->score ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}