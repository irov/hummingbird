#include "hb_grid_process_getleaderboard.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

hb_result_t hb_grid_process_getleaderboard( hb_grid_process_handle_t * _process, const hb_grid_process_getleaderboard_in_data_t * _in, hb_grid_process_getleaderboard_out_data_t * _out )
{
    HB_UNUSED( _out );

    if( hb_cache_expire_value( _process->cache, _in->token, sizeof( _in->token ), 1800 ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    hb_user_token_handle_t token_handle;
    if( hb_cache_get_value( _process->cache, _in->token, sizeof( _in->token ), &token_handle, sizeof( token_handle ), HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    if( hb_leaderboard_get( _process->cache, token_handle.poid, _in->begin, _in->end, HB_NULLPTR, HB_NULLPTR ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}