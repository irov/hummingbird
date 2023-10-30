#ifndef HB_GRID_PROCESS_SETUSERNICKNAME_H_
#define HB_GRID_PROCESS_SETUSERNICKNAME_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_setusernickname_in_data_t
{
    hb_uid_t user_uid;
    hb_uid_t project_uid;

    char nickname[32 + 1];
} hb_grid_process_setusernickname_in_data_t;

typedef struct hb_grid_process_setusernickname_out_data_t
{
    uint32_t dummy;
} hb_grid_process_setusernickname_out_data_t;

hb_result_t hb_grid_process_setusernickname( hb_grid_process_handle_t * _process, const hb_grid_process_setusernickname_in_data_t * _in, hb_grid_process_setusernickname_out_data_t * const _out );

#endif
