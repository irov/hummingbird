#ifndef HB_GRID_PROCESS_POSTMESSAGESCHANNEL_H_
#define HB_GRID_PROCESS_POSTMESSAGESCHANNEL_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_postmessageschannel_in_data_t
{
    hb_token_t token;

    hb_uid_t cuid;

    char message[256];
    char metainfo[256];
} hb_grid_process_postmessageschannel_in_data_t;

typedef struct hb_grid_process_postmessageschannel_out_data_t
{
    uint32_t postid;
    hb_error_code_t code;
} hb_grid_process_postmessageschannel_out_data_t;

hb_result_t hb_grid_process_postmessageschannel( hb_grid_process_handle_t * _process, const hb_grid_process_postmessageschannel_in_data_t * _in, hb_grid_process_postmessageschannel_out_data_t * _out );

#endif
