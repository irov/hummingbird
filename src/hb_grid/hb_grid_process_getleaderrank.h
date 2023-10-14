#ifndef HB_GRID_PROCESS_GETLEADERRANK_H_
#define HB_GRID_PROCESS_GETLEADERRANK_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"
//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_process_getleaderrank_in_data_t
{
    hb_uid_t user_uid;
    hb_uid_t project_uid;

} hb_grid_process_getleaderrank_in_data_t;
//////////////////////////////////////////////////////////////////////////
typedef struct hb_grid_process_getleaderrank_out_data_t
{
    uint32_t rank;
    hb_bool_t exist;

} hb_grid_process_getleaderrank_out_data_t;
//////////////////////////////////////////////////////////////////////////
hb_result_t hb_grid_process_getleaderrank( hb_grid_process_handle_t * _process, const hb_grid_process_getleaderrank_in_data_t * _in, hb_grid_process_getleaderrank_out_data_t * const _out );
//////////////////////////////////////////////////////////////////////////
#endif
