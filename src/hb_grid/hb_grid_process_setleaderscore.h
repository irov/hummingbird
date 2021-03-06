#ifndef HB_GRID_PROCESS_SETLEADERSCORE_H_
#define HB_GRID_PROCESS_SETLEADERSCORE_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_setleaderscore_in_data_t
{
    hb_uid_t uuid;
    hb_uid_t puid;

    uint32_t score;

} hb_grid_process_setleaderscore_in_data_t;

typedef struct hb_grid_process_setleaderscore_out_data_t
{
    uint32_t dummy;
} hb_grid_process_setleaderscore_out_data_t;

hb_result_t hb_grid_process_setleadscore( hb_grid_process_handle_t * _process, const hb_grid_process_setleaderscore_in_data_t * _in, hb_grid_process_setleaderscore_out_data_t * _out );

#endif
