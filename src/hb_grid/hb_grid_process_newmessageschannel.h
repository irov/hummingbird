#ifndef HB_GRID_PROCESS_NEWMESSAGESCHANNEL_H_
#define HB_GRID_PROCESS_NEWMESSAGESCHANNEL_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_newmessageschannel_in_data_t
{
    hb_uid_t auid;
    hb_uid_t puid;

    uint32_t maxpost;
} hb_grid_process_newmessageschannel_in_data_t;

typedef struct hb_grid_process_newmessageschannel_out_data_t
{
    hb_uid_t cuid;
} hb_grid_process_newmessageschannel_out_data_t;

hb_result_t hb_grid_process_newmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_newmessageschannel_in_data_t * _in, hb_grid_process_newmessageschannel_out_data_t * _out );

#endif
