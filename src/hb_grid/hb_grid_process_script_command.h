#ifndef HB_GRID_PROCESS_API_H_
#define HB_GRID_PROCESS_API_H_

#include "hb_config/hb_config.h"

#include "hb_grid.h"

typedef struct hb_grid_process_script_command_in_data_t
{
    hb_token_t token;

    hb_oid_t poid;

    char method[32];

    hb_data_t data;
    size_t data_size;

} hb_grid_process_script_command_in_data_t;

typedef struct hb_grid_process_script_command_out_data_t
{
    hb_bool_t successful;
    hb_bool_t method_found;

    char response_data[HB_GRID_REQUEST_DATA_MAX_SIZE];
    size_t response_size;

    size_t memory_used;
    uint32_t call_used;

} hb_grid_process_script_command_out_data_t;

hb_result_t hb_grid_process_script_command( hb_grid_process_handle_t * _process, const hb_grid_process_script_command_in_data_t * _in, hb_grid_process_script_command_out_data_t * _out );

#endif
