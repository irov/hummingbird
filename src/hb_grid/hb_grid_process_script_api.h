#ifndef HB_GRID_PROCESS_SCRIPT_API_H_
#define HB_GRID_PROCESS_SCRIPT_API_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_script_api_in_data_t
{
    hb_uid_t puid;
    hb_uid_t uuid;

    char api[32];
    char method[32];

    hb_json_handle_t * json_handle;

} hb_grid_process_script_api_in_data_t;

typedef struct hb_grid_process_script_api_out_data_t
{
    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_size;

    size_t memory_used;
    uint32_t call_used;

    hb_error_code_t code;
} hb_grid_process_script_api_out_data_t;

hb_result_t hb_grid_process_script_api( hb_grid_process_handle_t * _process, const hb_grid_process_script_api_in_data_t * _in, hb_grid_process_script_api_out_data_t * _out );

#endif
