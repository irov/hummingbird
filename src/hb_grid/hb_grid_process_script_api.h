#ifndef HB_GRID_PROCESS_SCRIPT_API_H_
#define HB_GRID_PROCESS_SCRIPT_API_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_script_api_in_data_t
{
    hb_uid_t project_uid;
    hb_uid_t user_uid;

    char api[32 + 1];
    char method[32 + 1];

    hb_json_handle_t * json_args;

} hb_grid_process_script_api_in_data_t;

typedef struct hb_grid_process_script_api_out_data_t
{
    char response_data[HB_GRID_RESPONSE_DATA_MAX_SIZE];
    hb_size_t response_size;

    hb_size_t memory_used;
    uint32_t call_used;

    hb_error_code_t code;
} hb_grid_process_script_api_out_data_t;

hb_result_t hb_grid_process_script_api( hb_grid_process_handle_t * _process, const hb_grid_process_script_api_in_data_t * _in, hb_grid_process_script_api_out_data_t * const _out );

#endif
