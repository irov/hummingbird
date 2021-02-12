#include "hb_grid_process_setleaderscore.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

hb_result_t hb_grid_process_setleadscore( hb_grid_process_handle_t * _process, const hb_grid_process_setleaderscore_in_data_t * _in, hb_grid_process_setleaderscore_out_data_t * _out )
{
    HB_UNUSED( _out );

    if( hb_leaderboard_set( _process->db_client, _process->cache, _in->puid, _in->uuid, _in->score ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    return HB_SUCCESSFUL;
}