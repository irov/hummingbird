#ifndef HB_GRID_PROCESS_NEWPROJECT_H_
#define HB_GRID_PROCESS_NEWPROJECT_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_newproject_in_data_t
{
    hb_token_t token;
} hb_grid_process_newproject_in_data_t;

typedef struct hb_grid_process_newproject_out_data_t
{
    hb_pid_t puid;
} hb_grid_process_newproject_out_data_t;

hb_result_t hb_grid_process_newproject( hb_grid_process_handle_t * _process, const hb_grid_process_newproject_in_data_t * _in, hb_grid_process_newproject_out_data_t * _out );

#endif
