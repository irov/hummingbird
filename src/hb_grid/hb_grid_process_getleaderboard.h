#ifndef HB_GRID_PROCESS_GETLEADERBOARD_H_
#define HB_GRID_PROCESS_GETLEADERBOARD_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_process_getleaderboard_in_data_t
{
    hb_uid_t user_uid;
    hb_uid_t project_uid;

    uint32_t leaderboard_begin;
    uint32_t leaderboard_end;

} hb_grid_process_getleaderboard_in_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_getleaderboard_desc_t
{
    uint64_t score;
    hb_uid_t user_uid;
    char nickname[32];
} hb_getleaderboard_desc_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_process_getleaderboard_out_data_t
{
    uint32_t descs_count;
    hb_getleaderboard_desc_t descs[16];

} hb_grid_process_getleaderboard_out_data_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_getleaderboard( hb_grid_process_handle_t * _process, const hb_grid_process_getleaderboard_in_data_t * _in, hb_grid_process_getleaderboard_out_data_t * const _out );
//////////////////////////////////////////////////////////////////////////

#endif
