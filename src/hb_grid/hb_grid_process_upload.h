#ifndef HB_GRID_PROCESS_UPLOAD_H_
#define HB_GRID_PROCESS_UPLOAD_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_upload_in_data_t
{
    hb_uid_t account_uid;
    hb_uid_t project_uid;

    const void * script_source;
    hb_size_t script_source_size;
} hb_grid_process_upload_in_data_t;

typedef struct hb_grid_process_upload_out_data_t
{
    uint32_t dummy;
} hb_grid_process_upload_out_data_t;

hb_result_t hb_grid_process_upload( hb_grid_process_handle_t * _process, const hb_grid_process_upload_in_data_t * _in, hb_grid_process_upload_out_data_t * const _out );

#endif
