#include "hb_grid_process_getleaderrank.h"

#include "hb_log/hb_log.h"
#include "hb_db/hb_db.h"
#include "hb_leaderboard/hb_leaderboard.h"

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_getleaderrank( hb_grid_process_handle_t * _process, const hb_grid_process_getleaderrank_in_data_t * _in, hb_grid_process_getleaderrank_out_data_t * const _out )
{
    HB_UNUSED( _out );

    uint32_t rank;
    hb_bool_t exist;
    if( hb_leaderboard_get_rank( _process->cache, _in->project_uid, _in->user_uid, &rank, &exist ) == HB_FAILURE )
    {
        return HB_FAILURE;
    }

    _out->rank = rank;
    _out->exist = exist;

    return HB_SUCCESSFUL;
}