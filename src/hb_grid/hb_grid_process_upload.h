#ifndef HB_GRID_PROCESS_UPLOAD_H_
#define HB_GRID_PROCESS_UPLOAD_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_upload_in_data_t
{
    hb_token_t token;

    hb_pid_t pid;

    hb_source_t script_source;
    size_t script_source_size;
} hb_grid_process_upload_in_data_t;

typedef struct hb_grid_process_upload_out_data_t
{
    int64_t revision;
    uint32_t version;
} hb_grid_process_upload_out_data_t;

hb_result_t hb_grid_process_upload( hb_grid_process_handle_t * _process, const hb_grid_process_upload_in_data_t * _in, hb_grid_process_upload_out_data_t * _out );

#endif
